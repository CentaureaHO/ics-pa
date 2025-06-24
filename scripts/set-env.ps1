$ScriptPath = $PSScriptRoot

$IcsHome = (Split-Path -Path $ScriptPath -Parent).Replace('\', '/')

$env:NEMU_HOME = "$IcsHome/nemu"
$env:AM_HOME = "$IcsHome/nexus-am"
$env:NAVY_HOME = "$IcsHome/navy-apps"
$env:ICS_HOME = $IcsHome

[System.Environment]::SetEnvironmentVariable('NEMU_HOME', $env:NEMU_HOME, 'User')
[System.Environment]::SetEnvironmentVariable('AM_HOME', $env:AM_HOME, 'User')
[System.Environment]::SetEnvironmentVariable('NAVY_HOME', $env:NAVY_HOME, 'User')
[System.Environment]::SetEnvironmentVariable('ICS_HOME', $env:ICS_HOME, 'User')
