#!/usr/bin/env python3
"""Download-entry fixtures: no network, real card, or production host overrides.

The optional --package test uses an existing full ZIP and its native Linux
installer with Cancel only; it never claims device/Mac execution acceptance.
"""
import argparse
import hashlib
import os
from pathlib import Path
import pty
import select
import shutil
import stat
import subprocess
import tempfile
import time
import unittest
import zipfile

ROOT = Path(__file__).resolve().parents[1]
REAL_PACKAGE = None


class OnlineInstallTest(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix="bf-online-test-")
        self.root = Path(self.temp.name)
        self.home = self.root / "computer home"
        self.home.mkdir()
        self.bin = self.root / "bin"
        self.bin.mkdir()
        self.card = self.root / "mounted cards" / "MIYOO"
        self.make_card(self.card)
        self.assets = self.root / "assets"
        self.assets.mkdir()
        self.script = self.root / "install-online.sh"
        # Tests substitute probes and mount locations in this isolated copy.
        code = (ROOT / "scripts/install-online.sh").read_text()
        code = code.replace("/usr/bin/sw_vers", str(self.bin / "sw_vers"))
        code = code.replace("/usr/sbin/sysctl", str(self.bin / "sysctl"))
        code = code.replace("roots=(/Volumes/*)", 'roots=("$TEST_ROOT"/"mounted cards"/*)')
        code = code.replace("roots=(/media/*/* /run/media/*/* /mnt/* /media/*)", 'roots=("$TEST_ROOT"/"mounted cards"/*)')
        self.script.write_text(code)
        self.env = dict(os.environ, HOME=str(self.home), PATH=str(self.bin) + ":" + os.environ["PATH"],
                        TEST_ROOT=str(self.root), TEST_OS="Linux", TEST_ARCH="x86_64",
                        TEST_TRANSLATED="0", TEST_VERSION="12.7", TEST_ASSETS=str(self.assets),
                        TEST_LOG=str(self.root / "calls"), TERM="dumb")
        self.tool("uname", 'case "$1" in -s) echo "$TEST_OS";; -m) echo "$TEST_ARCH";; -r) echo 6.8.0;; esac')
        self.tool("sw_vers", 'echo "$TEST_VERSION"')
        self.tool("sysctl", '[ "$TEST_TRANSLATED" != absent ] || exit 1; echo "$TEST_TRANSLATED"')
        self.tool("curl", '''[ "${TEST_DOWNLOAD_FAIL:-0}" != 1 ] || exit 22
dest=""; url=""
while [ "$#" -gt 0 ]; do
  if [ "$1" = --output ]; then dest=$2; shift 2; else url=$1; shift; fi
done
cp "$TEST_ASSETS/${url##*/}" "$dest"''')
        self.make_zip()

    def tearDown(self):
        self.temp.cleanup()

    def tool(self, name, body):
        p = self.bin / name
        p.write_text("#!/bin/sh\nset -eu\n" + body + "\n")
        p.chmod(0o755)

    def make_card(self, path):
        for name in ("App", "Roms", ".tmp_update/onionVersion", ".tmp_update/bin"):
            (path / name).mkdir(parents=True, exist_ok=True)
        (path / ".tmp_update/onionVersion/version.txt").write_text("v4.3.1-1\n")
        (path / ".tmp_update/runtime.sh").write_text("#!/bin/sh\n")
        elf = bytearray(52)
        elf[:7] = b"\x7fELF\x01\x01\x01"
        elf[18] = 40
        (path / ".tmp_update/bin/MainUI-354-clean").write_bytes(elf)

    def make_zip(self, extra=None, missing_layout=False):
        p = self.assets / "better-favorites-1.0.0-rc.7.zip"
        with zipfile.ZipFile(p, "w", zipfile.ZIP_DEFLATED) as z:
            if not missing_layout:
                base = "App/BetterFavorites/computer/"
                for entry in ("Install-macOS.command", "Install-Linux.sh"):
                    body = '#!/bin/sh\nprintf "%s\\n" "$@" > "$TEST_LOG"\nexit "${TEST_CHILD_STATUS:-0}"\n'
                    info = zipfile.ZipInfo(base + entry)
                    info.external_attr = (stat.S_IFREG | 0o755) << 16
                    z.writestr(info, body)
                for name in ("transport.json", "package.json"):
                    info = zipfile.ZipInfo(base + name)
                    info.external_attr = (stat.S_IFREG | 0o644) << 16
                    z.writestr(info, "{}")
            if extra:
                for name, data, mode in extra:
                    info = zipfile.ZipInfo(name)
                    info.create_system = 3
                    info.external_attr = mode << 16
                    z.writestr(info, data)
        self.checksum()

    def checksum(self):
        p = self.assets / "better-favorites-1.0.0-rc.7.zip"
        self.digest = hashlib.sha256(p.read_bytes()).hexdigest()
        (self.assets / "SHA256SUMS").write_text(self.digest + "  " + p.name + "\n")

    def run_entry(self, args=(), reply="y\n", piped=False):
        # A PTY supplies /dev/tty while stdin can be a piped script, as in curl|bash.
        master, slave = pty.openpty()
        command = ["bash", str(self.script), *args]
        if piped:
            command = ["bash", "-c", 'cat "$1" | bash -s -- "${@:2}"', "test", str(self.script), *args]
        def terminal():
            import fcntl
            import termios
            os.setsid()
            fcntl.ioctl(slave, termios.TIOCSCTTY, 0)
        before = sorted((str(p.relative_to(self.card)), p.read_bytes()) for p in self.card.rglob("*") if p.is_file())
        proc = subprocess.Popen(command, stdin=slave, stdout=slave, stderr=slave, env=self.env, preexec_fn=terminal)
        os.close(slave)
        os.write(master, reply.encode())
        output = bytearray()
        end = time.monotonic() + 30
        while time.monotonic() < end:
            if select.select([master], [], [], 0.1)[0]:
                try:
                    data = os.read(master, 65536)
                except OSError:
                    break
                if not data:
                    break
                output.extend(data)
            if proc.poll() is not None:
                # Drain remaining PTY output on the next iterations.
                if not select.select([master], [], [], 0.1)[0]:
                    break
        if proc.poll() is None:
            proc.kill()
            self.fail("entry did not finish: " + output.decode(errors="replace"))
        status = proc.wait()
        if status:
            for log in self.home.glob("BetterFavorites-Logs/*.log"):
                output.extend(log.read_bytes())
        os.close(master)
        after = sorted((str(p.relative_to(self.card)), p.read_bytes()) for p in self.card.rglob("*") if p.is_file())
        self.assertEqual(before, after, "download entry changed the card")
        return status, output.decode(errors="replace")

    def test_linux_native_menu_and_retention(self):
        status, out = self.run_entry()
        self.assertEqual(status, 0, out)
        self.assertIn("Linux x64", out)
        self.assertEqual((self.root / "calls").read_text().splitlines()[0:2], ["--staged-card", str(self.card)])
        self.assertEqual(len(list(self.home.glob("BetterFavorites-Downloads/*/package"))), 1)

    def test_linux_arm64_and_action_forwarding(self):
        self.env["TEST_ARCH"] = "aarch64"
        status, out = self.run_entry(["uninstall"])
        self.assertEqual(status, 0, out)
        self.assertIn("Linux ARM64", out)
        self.assertEqual((self.root / "calls").read_text().splitlines()[-1], "uninstall")

    def test_intel_without_optional_translation_key(self):
        self.env.update(TEST_OS="Darwin", TEST_TRANSLATED="absent")
        status, out = self.run_entry()
        self.assertEqual(status, 0, out)
        self.assertIn("macOS Intel", out)

    def test_native_apple_silicon(self):
        self.env.update(TEST_OS="Darwin", TEST_ARCH="arm64")
        status, out = self.run_entry()
        self.assertEqual(status, 0, out)
        self.assertIn("macOS Apple Silicon", out)

    def test_rosetta_selects_apple_silicon(self):
        self.env.update(TEST_OS="Darwin", TEST_TRANSLATED="1")
        status, out = self.run_entry()
        self.assertEqual(status, 0, out)
        self.assertIn("macOS Apple Silicon", out)

    def test_unsupported_hosts_and_old_macos(self):
        for os_name, arch, version in (("Windows", "x86_64", "12"), ("Linux", "i686", "12"), ("Darwin", "x86_64", "11.7")):
            self.env.update(TEST_OS=os_name, TEST_ARCH=arch, TEST_VERSION=version)
            status, out = self.run_entry()
            self.assertNotEqual(status, 0, out)
        self.assertFalse((self.root / "calls").exists())

    def test_cancel_before_download(self):
        status, out = self.run_entry(reply="n\n")
        self.assertEqual(status, 0, out)
        self.assertFalse((self.home / "BetterFavorites-Downloads").exists())

    def test_multiple_cards_require_selection(self):
        self.make_card(self.card.parent / "OTHER")
        status, out = self.run_entry(reply=str(self.card) + "\ny\n")
        self.assertEqual(status, 0, out)
        self.assertIn("More than one", out)

    def test_explicit_card_with_spaces(self):
        status, out = self.run_entry(["--sd-root", str(self.card)])
        self.assertEqual(status, 0, out)

    def test_no_card_prompts_for_path(self):
        self.script.write_text(self.script.read_text().replace('roots=("$TEST_ROOT"/"mounted cards"/*)', 'roots=("$TEST_ROOT"/missing/*)'))
        status, out = self.run_entry(reply=str(self.card) + "\ny\n")
        self.assertEqual(status, 0, out)

    def test_pipe_preserves_terminal_input(self):
        status, out = self.run_entry(piped=True)
        self.assertEqual(status, 0, out)

    def test_missing_or_duplicate_checksum(self):
        for text in ("0" * 64 + "  wrong.zip\n", (self.digest + "  better-favorites-1.0.0-rc.7.zip\n") * 2):
            (self.assets / "SHA256SUMS").write_text(text)
            status, out = self.run_entry()
            self.assertNotEqual(status, 0, out)
        self.assertFalse((self.root / "calls").exists())

    def test_corrupt_download(self):
        with (self.assets / "better-favorites-1.0.0-rc.7.zip").open("ab") as f:
            f.write(b"changed")
        status, out = self.run_entry()
        self.assertNotEqual(status, 0, out)
        self.assertIn("checksum mismatch", out)

    def test_download_failure_offers_connection_or_offline_help(self):
        self.env["TEST_DOWNLOAD_FAIL"] = "1"
        status, out = self.run_entry()
        self.assertNotEqual(status, 0, out)
        self.assertIn("check your connection and release tag", out)
        self.assertIn("offline ZIP", out)

    def test_authenticated_private_release(self):
        # Use a readable argument parser, rather than shell positional assumptions.
        self.tool("gh", 'dest=""; while [ "$#" -gt 0 ]; do if [ "$1" = --dir ]; then dest=$2; shift 2; else shift; fi; done; cp "$TEST_ASSETS/"* "$dest/"')
        self.env["TEST_DOWNLOAD_FAIL"] = "1"
        status, out = self.run_entry(["--github-auth"])
        self.assertEqual(status, 0, out)

    def test_unsafe_zip_members(self):
        for name, mode in (("../escape", stat.S_IFREG | 0o644), ("/absolute", stat.S_IFREG | 0o644), ("safe/link", stat.S_IFLNK | 0o777), ("safe\\escape", stat.S_IFREG | 0o644), ("safe/../escape", stat.S_IFREG | 0o644), ("name\nnewline", stat.S_IFREG | 0o644)):
            self.make_zip([(name, "target", mode)])
            status, out = self.run_entry()
            self.assertNotEqual(status, 0, (name, out))
        self.assertFalse((self.root / "calls").exists())

    def test_duplicate_zip_member(self):
        import warnings
        with warnings.catch_warnings():
            warnings.simplefilter("ignore", UserWarning)
            self.make_zip([("duplicate", "a", stat.S_IFREG | 0o644), ("duplicate", "b", stat.S_IFREG | 0o644)])
        status, out = self.run_entry()
        self.assertNotEqual(status, 0, out)
        self.assertIn("duplicate ZIP paths", out)

    def test_diagnostics_action_forwarding(self):
        status, out = self.run_entry(["export-diagnostics"])
        self.assertEqual(status, 0, out)
        self.assertEqual((self.root / "calls").read_text().splitlines()[-1], "export-diagnostics")

    def test_missing_full_layout(self):
        self.make_zip([("README.txt", "source is not the installer", stat.S_IFREG | 0o644)], missing_layout=True)
        status, out = self.run_entry()
        self.assertNotEqual(status, 0, out)
        self.assertIn("full installer layout", out)

    def test_child_failure_propagates(self):
        self.env["TEST_CHILD_STATUS"] = "37"
        status, out = self.run_entry()
        self.assertEqual(status, 37, out)

    def test_invalid_tag_and_symlinked_store(self):
        status, out = self.run_entry(["--tag", "../../bad"])
        self.assertNotEqual(status, 0, out)
        (self.home / "BetterFavorites-Downloads").symlink_to(self.assets)
        status, out = self.run_entry()
        self.assertNotEqual(status, 0, out)

    def test_existing_package_native_linux_cancel(self):
        if not REAL_PACKAGE:
            self.skipTest("provide --package for existing full-ZIP/native-Linux Cancel test")
        shutil.copyfile(REAL_PACKAGE, self.assets / "better-favorites-1.0.0-rc.7.zip")
        self.checksum()
        status, out = self.run_entry(reply="y\n0\n")
        self.assertEqual(status, 0, out)
        self.assertIn("Install / Update", out)
        self.assertIn("Uninstall completely", out)
        self.assertFalse((self.root / "calls").exists())


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--package", type=Path)
    options, unittest_args = parser.parse_known_args()
    REAL_PACKAGE = options.package
    unittest.main(argv=[__file__, *unittest_args])
