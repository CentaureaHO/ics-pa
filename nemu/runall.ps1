param()

$nemu = "build\nemu.exe"

Write-Host "Compiling NEMU..." -ForegroundColor Yellow
$makeResult = & make 2>&1
if ($LASTEXITCODE -ne 0) {
    Write-Host "NEMU compile error... exit..." -ForegroundColor Red
    exit 1
} else {
    Write-Host "NEMU compile OK" -ForegroundColor Green
}

Write-Host "Compiling testcases..." -ForegroundColor Yellow

$testSources = Get-ChildItem -Path "$env:AM_HOME\tests\cputest\tests" -Filter "*.c" | ForEach-Object { $_.BaseName }
$totalTests = $testSources.Count
$completedTests = 0

Write-Host "Found $totalTests test cases to compile" -ForegroundColor Cyan

foreach ($testName in $testSources) {
    $percentComplete = [math]::Round(($completedTests / $totalTests) * 100, 1)
    $remaining = $totalTests - $completedTests
    
    Write-Progress -Activity "Compiling testcases" -Status "Compiling $testName... ($completedTests/$totalTests completed, $remaining remaining)" -PercentComplete $percentComplete
    
    $makeResult = & make -C "$env:AM_HOME\tests\cputest" ARCH=x86-nemu ALL=$testName 2>&1
    if ($LASTEXITCODE -ne 0) {
        Write-Progress -Activity "Compiling testcases" -Completed
        Write-Host "testcases compile error for $testName... exit..." -ForegroundColor Red
        Write-Host $makeResult -ForegroundColor Red
        exit 1
    }
    
    $completedTests++
}

Write-Progress -Activity "Compiling testcases" -Status "Completed $completedTests/$totalTests test cases" -PercentComplete 100
Start-Sleep -Milliseconds 500
Write-Progress -Activity "Compiling testcases" -Completed

Write-Host "testcases compile OK ($completedTests/$totalTests completed)" -ForegroundColor Green

$binPath = "$env:AM_HOME\tests\cputest\build"
if (!(Test-Path $binPath)) {
    Write-Host "Build directory not found: $binPath" -ForegroundColor Red
    exit 1
}

$testBinaries = Get-ChildItem -Path $binPath -Filter "*-x86-nemu.bin" -ErrorAction SilentlyContinue
if ($testBinaries.Count -eq 0) {
    Write-Host "No test binaries found in $binPath" -ForegroundColor Red
    exit 1
}

Write-Host "Found $($testBinaries.Count) test binaries" -ForegroundColor Cyan

$oriLog = "build\nemu-log.txt"

Write-Host "`nRunning tests..." -ForegroundColor Yellow

foreach ($file in $testBinaries) {
    $base = $file.BaseName -replace "-x86-nemu", ""
    $logfile = "$base-log.txt"
    
    $paddedBase = $base.PadLeft(14)
    Write-Host "[$paddedBase] " -NoNewline -ForegroundColor Cyan
    
    & $nemu -b -l $oriLog $file.FullName *> $logfile 2>&1
    
    $success = Select-String -Path $logfile -Pattern "nemu: HIT GOOD TRAP" -Quiet
    if ($success) {
        Write-Host "PASS!" -ForegroundColor Green
        Remove-Item -Path $logfile -Force -ErrorAction SilentlyContinue
    } else {
        Write-Host "FAIL! see $logfile for more information" -ForegroundColor Red
        
        if (Test-Path $oriLog) {
            Add-Content -Path $logfile -Value "`n`n===== the original log.txt =====`n"
            Get-Content -Path $oriLog | Add-Content -Path $logfile
        }
    }
}

Write-Host "`nAll tests completed." -ForegroundColor Yellow
