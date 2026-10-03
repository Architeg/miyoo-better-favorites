# Deferred shortcut investigation — excluded from v1.0

All global shortcut combinations are deferred beyond v1.0. This report preserves
research evidence; its proposed next steps are historical candidates, not current
release work.

2026-10-03. This work is independent of the accepted Home redirect. **No shortcut
adapter, installer, setting, help row or deployed change is provided yet.** Home
replacement ON/OFF and Apps continue to work independently. The evidence below
shows why a settings-only chord or a keymon observer launching the app is unsafe.

## Verified source and mounted configuration

Pinned source read through GitHub MCP at Onion `v4.3.1-1`:

- [`keymon.c`](https://github.com/OnionUI/Onion/blob/v4.3.1-1/src/keymon/keymon.c),
  `main`: opens `/dev/input/event0`; Y on `PRESSED` calls
  `applyExtraButtonShortcut(1)`. X calls its own shortcut and clears `launch_alt`.
  There is no L1-held chord state in this loop. Source blob
  `abb746d126c62457f290e3059d9601e3561c71fc`.
- [`menuButtonAction.h`](https://github.com/OnionUI/Onion/blob/v4.3.1-1/src/keymon/menuButtonAction.h),
  `applyExtraButtonShortcut`: uses MainUI mode and separate configured X/Y actions.
  Y=`glo` sets `launch_alt`; app actions use `_action_runApp`, write a command,
  write MainUI state `(MAIN_MENU,0,10)`, then kill MainUI. That app path is not the
  native Home state-saving adapter. Blob `b3328b62c4dfe812a8aa6793cd7bcde7d7cdf971`.
- [`input_fd.h`](https://github.com/OnionUI/Onion/blob/v4.3.1-1/src/keymon/input_fd.h),
  `keyinput_isValid` reads its descriptor; the ignore queue ignores only keymon's
  own injected events. `keyinput_disable` uses EVIOCGRAB, with an unbounded retry;
  no L1+Y grab exists. Grabbing after Y has already arrived cannot retract MainUI's
  queued copy. Intercepting all input while L1 is held changes native L1 behavior
  and needs careful replay/release/focus qualification.
- [`keymap_hw.h`](https://github.com/OnionUI/Onion/blob/v4.3.1-1/src/common/system/keymap_hw.h):
  L1=`KEY_E`, Y=`KEY_LEFTALT`, X=`KEY_LEFTSHIFT`.
- [`settings.h`](https://github.com/OnionUI/Onion/blob/v4.3.1-1/src/common/system/settings.h),
  `_settings_load_keymap` loads independent X/Y strings from the config keymap.

Mounted `.tmp_update/config/keymap.json`: X=`app:Search`, Y=`glo`. No preferences
or global audio settings were changed. Mounted keymon SHA-256:
`297918f9368ff7b804e6675fc746be1f9e92e26ae911ee95af99f6cd5f07a206`.
That is a version-specific binary identity, not proof that rebuilding the tagged
source is byte-identical or a ready keymon patch allowlist.

## Exact MainUI input evidence

The four original MainUI hashes are the accepted Home allowlist. Read-only fixtures
verify these native instruction paths and that the accepted patch leaves their
entire `0x17744..0x17f20` range unchanged:

| Address | Native behavior established from instructions |
| --- | --- |
| `0x1775c` | SDL_WaitEvent, independent MainUI event delivery |
| `0x17948` / `0x17a64` | L1 key-down branch yields native action 14 |
| `0x17bdc` / `0x17c48` | X key-up yields action 3 |
| `0x17c08` / `0x17c30` | Y key-up yields action 4 |

L1 is not safely assumed inert. `quietMainUI` itself injects L1 to quiet native
MainUI; the complete context-specific action-14 effects and stale-key transitions
have not been qualified. Native Y release and keymon's Y press are separate
consumers. Skipping only one branch does not suppress both. Calling kill_mainUI
asynchronously after staging is not proof that a native Y action cannot run first.
Stock keymon's MainUI-mode classification and game/app transitions also need a
fresh process identity/focus check before any ownership decision.

## Existing command compatibility — a confirmed blocker

Tagged [`apps.h`](https://github.com/OnionUI/Onion/blob/v4.3.1-1/src/common/utils/apps.h),
`set_cmd_app`, emits the following one-space suffix for the current app:

```sh
cd /mnt/SDCARD/App/BetterFavoritesTest; chmod a+x ./launch.sh; LD_PRELOAD=/mnt/SDCARD/miyoo/app/../lib/libpadsp.so ./launch.sh
```

Our verified native Apps/Home path uses **three** spaces before `./launch.sh`.
`src/launch_request.cpp::validAppCommand` deliberately accepts only that exact
canonical form plus trailing whitespace. The separate compatibility fixture
passes the exact tagged one-space form through the actual A and MENU publication
functions: both refuse it and preserve the active file; no game/switcher flag is
queued. The fixture also runs as compiled ARM under qemu. This is a host/emulator
result, not device acceptance. Production validation was not relaxed.

`set_cmd_app` uses the stock truncating command writer; existence checks or later
readback cannot give it no-replace publication. `_action_runApp` saves an Apps
context, not the native Home-selected context. Reusing those functions unchanged
therefore neither establishes safe concurrent publication nor Home restoration.

## Decision and smallest next candidate

Do not prototype/deploy a one-sided Y observer or expose shortcut instructions.
The remaining input and state-restoration questions must be settled first:

1. Map action 14/4 across Home and other native MainUI contexts and key-up/repeat
   handling; compare focus/process transitions, shutdown and existing MENU combos.
2. Determine a narrowly scoped way to suppress **both** consumers for an actual
   L1-held-before-Y chord, while ordinary Y/X remain exact stock behavior. Options
   require coordinated MainUI/keymon changes or a fully qualified existing-input
   grab/replay scheme; neither is established as a small safe patch yet.
3. Dispatch through a native AppAction transaction with the already accepted
   canonical command and no-replace writer guard, or qualify a separate exact
   command form end-to-end. Do not broadly normalize arbitrary shell commands.
4. Choose and verify context saving for the requested shortcut. Independent
   installation must work against exact original MainUI **and** the exact reviewed
   Home-patched set, with version-specific composition and verified rollback. Home
   preference OFF must not disable the shortcut; no Home patch must be required
   merely as an installation dependency.
5. Only then build an opt-in compiled ARM prototype: single non-repeated chord,
   modifier reset on process/focus change, unavailable app, command conflicts,
   shutdown, failed allocation/publication, no duplicate action and proper return.

No new daemon is needed in a candidate that extends existing owners, but its
correctness/size/compatibility is not yet established. This finding is a blocker
to a safe prototype, not a claim that a shortcut is impossible. No separate tile
or entry inside stock Favorites is in scope. The accepted Home adapter, app
ownership checks, runtime return and global audio configuration remain unchanged.

## Reproducible checks (isolated host, no card writes)

```sh
python3 tests/mainui-home/shortcut_input_audit_test.py \
  build/m6-accepted-fixture/.tmp_update/bin build/m6-home-review-final/all-variants
c++ -std=c++17 -Wall -Wextra -Iinclude \
  tests/mainui-home/shortcut_compatibility_test.cpp src/launch_request.cpp \
  src/app_settings.cpp -o build/m6-shortcut-compatibility
build/m6-shortcut-compatibility
```

The fixture directory contains host-only verified copies from the original backup;
no vendor binaries are added to Git. The separate review diff contains this report
and these evidence tests only. No shortcut deployment or device test is requested.
