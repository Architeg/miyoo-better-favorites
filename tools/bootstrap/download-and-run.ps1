# Windows PowerShell 2-compatible preparation entry. Online TLS still needs native qualification.
param([string]$Action, [string]$Tag)
$ErrorActionPreference = 'Stop'
try {
    if ($Action -ne 'install' -and $Action -ne 'uninstall') { throw 'Use install or uninstall followed by an explicit published tag.' }
    if ($Tag -notmatch '^v[0-9]+\.[0-9]+\.[0-9]+(-[A-Za-z0-9.-]+)?$') { throw 'Explicit published tag required.' }
    if ([Environment]::OSVersion.Platform -ne [PlatformID]::Win32NT -or [Environment]::OSVersion.Version -lt [Version]'6.1') { throw 'Windows 7 or later required.' }
    # Process-local TLS selection; never relax certificates or change system policy.
    [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]3072
    $client = New-Object Net.WebClient
    $client.Headers.Add('User-Agent', 'BetterFavorites-bootstrap')
    $api = 'https://api.github.com/repos/Architeg/miyoo-better-favorites'
    $base = 'https://github.com/Architeg/miyoo-better-favorites/releases/download/' + $Tag + '/'
    $release = $client.DownloadString($api + '/releases/tags/' + $Tag)
    if ($release -notmatch '"draft"\s*:\s*false' -or $release -notmatch '"published_at"\s*:\s*"[0-9]') { throw 'Draft or unpublished release refused.' }
    if ($release -notmatch '"immutable"\s*:\s*true') { throw 'Immutable published release required.' }
    $name = 'better-favorites-bootstrap-windows-386.exe'
    $checks = $client.DownloadString($base + 'BOOTSTRAP-SHA256SUMS')
    $matches = @($checks -split "`n" | Where-Object { $_ -match ('^[a-f0-9]{64}  ' + [Regex]::Escape($name) + '\s*$') })
    if ($matches.Count -ne 1) { throw 'Missing or ambiguous bootstrap checksum.' }
    $expected = $matches[0].Substring(0,64)
    $store = Join-Path ([Environment]::GetFolderPath('UserProfile')) 'BetterFavorites-Downloads'
    $work = Join-Path $store ('bootstrap-' + $Tag + '-' + [Guid]::NewGuid().ToString('N'))
    [IO.Directory]::CreateDirectory($work) | Out-Null
    $exe = Join-Path $work $name
    $client.DownloadFile($base + $name, $exe)
    $hasher = [Security.Cryptography.SHA256]::Create()
    $stream = [IO.File]::OpenRead($exe)
    try { $actual = [BitConverter]::ToString($hasher.ComputeHash($stream)).Replace('-','').ToLowerInvariant() }
    finally { $stream.Dispose(); $hasher.Dispose(); $client.Dispose() }
    if ($actual -ne $expected) { throw 'Bootstrap checksum mismatch.' }
    & $exe $Action --tag $Tag --store $store -- @args
    exit $LASTEXITCODE
} catch {
    Write-Error ('Online bootstrap unavailable: ' + $_.Exception.Message + '. Use the offline extracted-package entry instead.') -ErrorAction Continue
    exit 1
}
