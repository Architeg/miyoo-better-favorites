# Maintained source for the literal built-in PowerShell command in Install-Windows.cmd.
# PowerShell 2 / .NET 2-compatible syntax. No policy override, encoded command,
# downloaded code, Add-Type compiler, or compiled dispatcher.
function Select-BetterFavoritesTool($version, $architectures, $nativeBits) {
    $v = [Version]$version;
    $family = '';
    if ($v.Major -eq 6 -and $v.Minor -ge 1 -and $v.Minor -le 3 -and $v.Build -gt 0) { $family = 'windows7'; };
    if ($v.Major -eq 10 -and $v.Minor -eq 0 -and $v.Build -ge 10240) { $family = 'windows'; };
    $native = @($architectures | Select-Object -Unique);
    if ($family -eq '' -or $native.Count -ne 1) { throw 'Unsupported or ambiguous Windows version/architecture.'; };
    $arch = '';
    switch ([int]$native[0]) { 0 { $arch = '386'; } 9 { $arch = 'amd64'; } 12 { if ($family -eq 'windows') { $arch = 'arm64'; }; } };
    if ($nativeBits -eq 32 -and ([int]$native[0] -eq 0 -or [int]$native[0] -eq 9)) { $arch = '386'; } elseif ($nativeBits -ne 64 -or $arch -eq '386') { throw 'Contradictory native Windows process/processor identity.'; };
    if ($arch -eq '') { throw 'Unsupported native Windows architecture.'; };
    return 'better-favorites-installer-' + $family + '-' + $arch + '.exe';
};
function Assert-BetterFavoritesPath($path, $directory, $limit) {
    $full = [IO.Path]::GetFullPath($path);
    $item = Get-Item -LiteralPath $full -Force -ErrorAction Stop;
    if ([bool]$item.PSIsContainer -ne [bool]$directory) { throw ('Wrong file type: ' + $full); };
    if (-not $directory -and $item.Length -gt $limit) { throw ('Oversized input: ' + $full); };
    $parent = $full;
    while ($parent) {
        if (([IO.File]::GetAttributes($parent) -band [IO.FileAttributes]::ReparsePoint) -ne 0) { throw ('Link/junction preserved: ' + $parent); };
        $parent = [IO.Path]::GetDirectoryName($parent);
    };
    return $full;
};
function Get-BetterFavoritesHash($path) {
    $stream = [IO.File]::Open($path, [IO.FileMode]::Open, [IO.FileAccess]::Read, [IO.FileShare]::Read);
    $hash = [Security.Cryptography.SHA256]::Create();
    try { if ($stream.Length -gt 67108864) { throw 'Oversized executable.'; }; return [BitConverter]::ToString($hash.ComputeHash($stream)).Replace('-','').ToLowerInvariant(); }
    finally { $stream.Close(); $hash.Clear(); };
};
function Prepare-BetterFavoritesTool($tools, $stage, $name, $copy) {
    $tools = Assert-BetterFavoritesPath $tools $true 0;
    $stage = Assert-BetterFavoritesPath $stage $true 0;
    if ($name -notmatch '^better-favorites-installer-(windows7|windows)-(386|amd64|arm64)\.exe$') { throw 'Invalid installer selection.'; };
    $manifest = Assert-BetterFavoritesPath (Join-Path $tools 'HOST-SHA256SUMS') $false 16384;
    $expected = '';
    foreach ($line in [IO.File]::ReadAllLines($manifest)) {
        if ($line -notmatch '^([a-f0-9]{64})  ([A-Za-z0-9._-]+)$') { throw 'Invalid host checksum inventory.'; };
        if ($matches[2] -eq $name) { if ($expected) { throw 'Duplicate host checksum.'; }; $expected = $matches[1]; };
    };
    if (-not $expected) { throw 'Selected installer is not in the package inventory.'; };
    $source = Assert-BetterFavoritesPath (Join-Path $tools $name) $false 67108864;
    if ((Get-BetterFavoritesHash $source) -ne $expected) { throw ('Installer checksum mismatch: ' + $name); };
    $destination = $source;
    if ($copy) {
        $destination = Join-Path $stage $name;
        [IO.File]::Copy($source, $destination, $false);
        $destination = Assert-BetterFavoritesPath $destination $false 67108864;
        if ((Get-BetterFavoritesHash $destination) -ne $expected) { throw 'Copied installer checksum mismatch.'; };
    };
    [IO.File]::WriteAllText((Join-Path $stage 'native-name.txt'), $name, [Text.Encoding]::ASCII);
    return $destination;
};
function Invoke-BetterFavoritesSelection {
    $ErrorActionPreference = 'Stop';
    try {
        if ($env:OS -ne 'Windows_NT') { throw 'Windows is required.'; };
        $os = @(Get-WmiObject -Class Win32_OperatingSystem -ErrorAction Stop);
        if ($os.Count -ne 1) { throw 'Ambiguous Windows identity.'; };
        $cpu = @(Get-WmiObject -Class Win32_Processor -ErrorAction Stop | ForEach-Object { $_.Architecture });
        $name = Select-BetterFavoritesTool $os[0].Version $cpu ([IntPtr]::Size*8);
        $copy = $env:BF_ENTRY_MODE -eq 'card';
        $null = Prepare-BetterFavoritesTool $env:BF_TOOL_ROOT $env:BF_EXEC_STAGE $name $copy;
        exit 0;
    } catch {
        [Console]::Error.WriteLine('Windows entry failed: ' + $_.Exception.Message);
        [Console]::Error.WriteLine('The installer did not run. No card changes were made. Keep the package and report this reason.');
        exit 2;
    };
};
