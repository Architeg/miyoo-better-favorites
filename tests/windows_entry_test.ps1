$ErrorActionPreference = 'Stop'
. /repo/packaging/windows-select.ps1
$cases = @(@('6.1.7601',0,'windows7-386'),@('6.1.7601',9,'windows7-amd64'),@('6.2.9200',9,'windows7-amd64'),@('6.3.9600',9,'windows7-amd64'),@('10.0.19045',9,'windows-amd64'),@('10.0.22631',12,'windows-arm64'),@('10.0.19045',0,'windows-386'))
foreach($c in $cases){$actual=Select-BetterFavoritesTool $c[0] @($c[1]) $(if($c[1] -eq 0){32}else{64});if($actual -ne ('better-favorites-installer-'+$c[2]+'.exe')){throw 'selection mismatch'}}
foreach($c in @(@('6.0.6002',9),@('6.1.7601',12),@('10.0.19045',5),@('11.0.30000',9))){$failed=$false;try{Select-BetterFavoritesTool $c[0] @($c[1]) 64}catch{$failed=$true};if(-not $failed){throw 'unsupported accepted'}}
try {Select-BetterFavoritesTool '10.0.19045' @(0,9) 64;throw 'ambiguous accepted'}catch{if($_.Exception.Message -eq 'ambiguous accepted'){throw}}
$base='/tmp/windows-entry-test';New-Item -ItemType Directory -Path ($base+'/tools'),($base+'/stage') -Force | Out-Null
$name='better-favorites-installer-windows7-amd64.exe';[IO.File]::WriteAllText($base+'/tools/'+$name,'native fixture bytes')
$hash=Get-BetterFavoritesHash ($base+'/tools/'+$name)
[IO.File]::WriteAllText($base+'/tools/HOST-SHA256SUMS',$hash+'  '+$name+[Environment]::NewLine)
$out=Prepare-BetterFavoritesTool ($base+'/tools') ($base+'/stage') $name $true
if((Get-BetterFavoritesHash $out) -ne $hash){throw 'copy mismatch'}
if([IO.File]::ReadAllText($base+'/stage/native-name.txt') -ne $name){throw 'name mismatch'}
$failed=$false;try{Prepare-BetterFavoritesTool ($base+'/tools') ($base+'/stage') $name $true}catch{$failed=$true};if(-not $failed){throw 'existing copy overwritten'}
[IO.File]::WriteAllText($base+'/tools/'+$name,'modified')
$failed=$false;try{Prepare-BetterFavoritesTool ($base+'/tools') ($base+'/stage') $name $false}catch{$failed=$true};if(-not $failed){throw 'bad hash accepted'}
$cmd=[IO.File]::ReadAllText('/repo/packaging/Install-Windows.cmd');$line=@($cmd -split "`n"|Where-Object{$_ -match ' -Command '})[0];$literal=$line.Substring($line.IndexOf(' -Command ')+11).Trim().Trim('"');$null=[ScriptBlock]::Create($literal)
Write-Output 'PASS: native version/architecture dispatch; ambiguity; hash/copy/failure; generated literal PowerShell syntax. Simulated on Linux PowerShell, not Windows acceptance.'

$null = New-Item -ItemType SymbolicLink -Path ($base+'/linked-tools') -Target ($base+'/tools')
$failed=$false;try{Assert-BetterFavoritesPath ($base+'/linked-tools') $true 0}catch{$failed=$true};if(-not $failed){throw 'reparse/link accepted'}
& (Get-Process -Id $PID).Path -NoLogo -NoProfile -Command ". /repo/packaging/windows-select.ps1; `$env:OS='Unsupported'; Invoke-BetterFavoritesSelection"
if($LASTEXITCODE -ne 2){throw 'entry failure status lost'}
Write-Output 'PASS: link refusal and actual selector failure exit status 2.'

if((Select-BetterFavoritesTool '6.1.7601' @(9) 32) -ne 'better-favorites-installer-windows7-386.exe'){throw '32-bit OS on x64 processor failed'}
if((Select-BetterFavoritesTool '10.0.19045' @(9) 64) -ne 'better-favorites-installer-windows-amd64.exe'){throw 'native PowerShell through Sysnative failed'}
Write-Output 'PASS: native OS bitness distinguished from CPU capability; 32-bit Windows on x64 CPU.'
