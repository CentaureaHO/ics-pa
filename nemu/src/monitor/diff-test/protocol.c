/* Simple implementation of a GDB remote protocol client.
 * Copyright (C) 2015 Red Hat Inc.
 *
 * This file is part of gdb-toys.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

 #include "common.h"
 #include <ctype.h>
 #include <stdbool.h>
 #include <stdint.h>
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 
 #ifdef _WIN32
 #include <winsock2.h>
 #include <ws2tcpip.h>
 #else
 #include <err.h>
 #include <unistd.h>
 #include <arpa/inet.h>
 #include <netinet/in.h>
 #include <netinet/tcp.h>
 #include <sys/socket.h>
 #include <sys/types.h>
 #endif
 
 #include "protocol.h"
 
 struct gdb_conn
 {
 #ifdef _WIN32
     SOCKET fd;
 #else
     FILE* in;
     FILE* out;
 #endif
     bool ack;
 };
 
 static uint8_t hex_nibble(uint8_t hex) { return isdigit(hex) ? hex - '0' : tolower(hex) - 'a' + 10; }
 
 uint8_t hex_encode(uint8_t digit) { return digit > 9 ? 'a' + digit - 10 : '0' + digit; }
 
 uint16_t gdb_decode_hex(uint8_t msb, uint8_t lsb)
 {
     if (!isxdigit(msb) || !isxdigit(lsb)) return UINT16_MAX;
     return 16 * hex_nibble(msb) + hex_nibble(lsb);
 }
 
 uint64_t gdb_decode_hex_str(uint8_t* bytes)
 {
     uint64_t val = 0;
     char tmp[3] = {0};
     uint8_t* p = bytes;
     int i = 0;
     while(isxdigit(p[0]) && isxdigit(p[1])) {
         tmp[0] = p[0];
         tmp[1] = p[1];
         uint8_t byte = strtol(tmp, NULL, 16);
         val |= ((uint64_t)byte) << (i * 8);
         p += 2;
         i++;
     }
     return val;
 }
 
 #ifdef _WIN32
 static struct gdb_conn* gdb_begin(SOCKET fd) {
     struct gdb_conn* conn = calloc(1, sizeof(struct gdb_conn));
     assert(conn != NULL);
 
     conn->ack = true;
     conn->fd = fd;
 
     char plus = '+';
     send(conn->fd, &plus, 1, 0);
 
     return conn;
 }
 #else
 static struct gdb_conn* gdb_begin(int fd)
 {
     struct gdb_conn* conn = calloc(1, sizeof(struct gdb_conn));
     if (conn == NULL) err(1, "calloc");
 
     conn->ack = true;
 
     // duplicate the handle to separate read/write state
     int fd2 = dup(fd);
     if (fd2 < 0) err(1, "dup");
 
     // open a FILE* for reading
     conn->in = fdopen(fd, "rb");
     if (conn->in == NULL) err(1, "fdopen");
 
     // open a FILE* for writing
     conn->out = fdopen(fd2, "wb");
     if (conn->out == NULL) err(1, "fdopen");
 
     // reset line state by acking any earlier input
     fputc('+', conn->out);
     fflush(conn->out);
 
     return conn;
 }
 #endif
 
 struct gdb_conn* gdb_begin_inet(const char* addr, uint16_t port)
 {
 #ifdef _WIN32
     WSADATA wsaData;
     if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
         perror("WSAStartup failed");
         return NULL;
     }
     SOCKET fd = socket(AF_INET, SOCK_STREAM, 0);
     if (fd == INVALID_SOCKET) {
         perror("socket");
         WSACleanup();
         return NULL;
     }
 #else
     int fd = socket(AF_INET, SOCK_STREAM, 0);
     if (fd < 0) err(1, "socket");
 #endif
 
     struct sockaddr_in sa = {
         .sin_family = AF_INET, .sin_port = htons(port),
     };
 
 #ifdef _WIN32
     inet_pton(AF_INET, addr, &sa.sin_addr);
 #else
     if (inet_aton(addr, &sa.sin_addr) == 0) errx(1, "Invalid address: %s", addr);
 #endif
 
     if (connect(fd, (const struct sockaddr*)&sa, sizeof(sa)) != 0) {
 #ifdef _WIN32
         closesocket(fd);
         WSACleanup();
 #else
         close(fd);
 #endif
         return NULL;
     }
 
 #ifdef _WIN32
     char tmp = 1;
     setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &tmp, sizeof(tmp));
     setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &tmp, sizeof(tmp));
 #else
     socklen_t tmp;
     tmp   = 1;
     int r = setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (char*)&tmp, sizeof(tmp));
     if (r) {
         perror("setsockopt");
         panic("setsockopt error");
     }
     tmp = 1;
     r   = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char*)&tmp, sizeof(tmp));
     if (r) {
         perror("setsockopt");
         panic("setsockopt error");
     }
 #endif
 
     return gdb_begin(fd);
 }
 
 void gdb_end(struct gdb_conn* conn)
 {
     if (!conn) return;
 #ifdef _WIN32
     closesocket(conn->fd);
     WSACleanup();
 #else
     fclose(conn->in);
     fclose(conn->out);
 #endif
     free(conn);
 }
 
 #ifdef _WIN32
 static void send_packet(SOCKET out, const uint8_t* command, size_t size) {
     uint8_t sum = 0;
     for (size_t i = 0; i < size; ++i) sum += command[i];
 
     char* buf = malloc(size + 5);
     assert(buf);
     buf[0] = '$';
     memcpy(buf + 1, command, size);
     int p = size + 1;
     p += sprintf(buf + p, "#%02x", sum);
 
     send(out, buf, p, 0);
     free(buf);
 }
 #else
 static void send_packet(FILE* out, const uint8_t* command, size_t size)
 {
     // compute the checksum -- simple mod256 addition
     uint8_t sum = 0;
     size_t  i;
     for (i = 0; i < size; ++i) sum += command[i];
 
     fputc('$', out);                // packet start
     fwrite(command, 1, size, out);  // payload
     fprintf(out, "#%02X", sum);     // packet end, checksum
     fflush(out);
 
     if (ferror(out))
         err(1, "send");
     else if (feof(out))
         errx(0, "send: Connection closed");
 }
 #endif
 
 void gdb_send(struct gdb_conn* conn, const uint8_t* command, size_t size)
 {
     bool acked = false;
     do
     {
 #ifdef _WIN32
         send_packet(conn->fd, command, size);
 #else
         send_packet(conn->out, command, size);
 #endif
 
         if (!conn->ack) break;
 
 #ifdef _WIN32
         char ack_char;
         int ret = recv(conn->fd, &ack_char, 1, 0);
         if (ret <= 0) {
             acked = false;
             break;
         }
         acked = (ack_char == '+');
 #else
         acked = fgetc(conn->in) == '+';
 #endif
     } while (!acked);
 }
 
 #ifdef _WIN32
 static uint8_t* recv_packet(SOCKET in, size_t* ret_size, bool* ret_sum_ok) {
     size_t size = 4096;
     uint8_t* reply = malloc(size);
     assert(reply);
 
     char c;
     do {
         if (recv(in, &c, 1, 0) <= 0) { free(reply); return NULL; }
     } while (c != '$');
 
     size_t i = 0;
     uint8_t sum = 0;
     while (1) {
         if (recv(in, &c, 1, 0) <= 0) { free(reply); return NULL; }
         if (c == '#') {
             break;
         }
         sum += c;
         if (i >= size) {
             size *= 2;
             reply = realloc(reply, size);
             assert(reply);
         }
         reply[i++] = c;
     }
 
     char sum_buf[2];
     if (recv(in, sum_buf, 2, 0) < 2) { free(reply); return NULL; }
     *ret_sum_ok = (sum == gdb_decode_hex(sum_buf[0], sum_buf[1]));
     *ret_size = i;
     reply[i] = '\0';
     return reply;
 }
 #else
 static uint8_t* recv_packet(FILE* in, size_t* ret_size, bool* ret_sum_ok)
 {
     size_t   i     = 0;
     size_t   size  = 4096;
     uint8_t* reply = malloc(size);
     if (reply == NULL) err(1, "malloc");
 
     int     c;
     uint8_t sum    = 0;
     bool    escape = false;
 
     // fast-forward to the first start of packet
     while ((c = fgetc(in)) != EOF && c != '$')
         ;
 
     while ((c = fgetc(in)) != EOF) {
         sum += c;
         switch (c)
         {
             case '$':  // new packet?  start over...
                 i      = 0;
                 sum    = 0;
                 escape = false;
                 continue;
 
             case '#':      // end of packet
                 sum -= c;  // not part of the checksum
                 {
                     uint8_t msb = fgetc(in);
                     uint8_t lsb = fgetc(in);
                     *ret_sum_ok = sum == gdb_decode_hex(msb, lsb);
                 }
                 *ret_size = i;
 
                 // terminate it for good measure
                 if (i == size) {
                     reply = realloc(reply, size + 1);
                     if (reply == NULL) err(1, "realloc");
                 }
                 reply[i] = '\000';
 
                 return reply;
 
             case '}':  // escape: next char is XOR 0x20
                 escape = true;
                 continue;
 
             case '*':  // run-length-encoding
                 if (i > 0) {  // need something to repeat!
                     int c2 = fgetc(in);
                     if (c2 < 29 || c2 > 126 || c2 == '$' || c2 == '#') {
                         ungetc(c2, in);
                     }
                     else
                     {
                         int count = c2 - 29;
                         if (i + count > size) {
                             size *= 2;
                             reply = realloc(reply, size);
                             if (reply == NULL) err(1, "realloc");
                         }
                         memset(&reply[i], reply[i - 1], count);
                         i += count;
                         sum += c2;
                         continue;
                     }
                 }
         }
 
         if (escape) {
             c ^= 0x20;
             escape = false;
         }
 
         if (i == size) {
             size *= 2;
             reply = realloc(reply, size);
             if (reply == NULL) err(1, "realloc");
         }
 
         reply[i++] = c;
     }
 
     if (ferror(in))
         err(1, "recv");
     else if (feof(in))
         errx(0, "recv: Connection closed");
     else
         errx(1, "recv: Unknown connection error");
     return NULL;
 }
 #endif
 
 uint8_t* gdb_recv(struct gdb_conn* conn, size_t* size)
 {
     uint8_t* reply;
     bool acked = false;
     do
     {
 #ifdef _WIN32
         reply = recv_packet(conn->fd, size, &acked);
 #else
         reply = recv_packet(conn->in, size, &acked);
 #endif
         if (!reply) break;
 
         if (!conn->ack) break;
 
 #ifdef _WIN32
         send(conn->fd, acked ? "+" : "-", 1, 0);
 #else
         fputc(acked ? '+' : '-', conn->out);
         fflush(conn->out);
 #endif
     } while (!acked);
 
     return reply;
 }
 
 const char* gdb_start_noack(struct gdb_conn* conn)
 {
     static const char cmd[] = "QStartNoAckMode";
     gdb_send(conn, (const uint8_t*)cmd, sizeof(cmd) - 1);
 
     size_t   size;
     uint8_t* reply = gdb_recv(conn, &size);
     bool     ok    = size == 2 && !strcmp((const char*)reply, "OK");
     free(reply);
 
     if (ok) conn->ack = false;
     return ok ? "OK" : "";
 }
 