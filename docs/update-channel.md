# The update channel of this fork

The clients built from this repository do not check `update.tdesktop.com`.
They poll a channel hosted on GitHub Pages:

```
https://bormisov1.github.io/tdesktop/current6
```

which is `Local::readAutoupdatePrefix() + "/current6"` — the prefix is
compiled in, so neither the `tdata/prefix` file nor the
`autoupdate_url_prefix` field of `help.getConfig` can move a client off it
(the latter is ignored in `mtp_instance.cpp` on purpose).

## The feed

`current6` is the same JSON the upstream feed serves, with only the Linux
platform present:

```json
{"linux":{"stable":{"released":7002011,"link":"td-update-linux-x64-7002011"}}}
```

`link` is relative to the prefix, and `{version}` in it is substituted with
the announced version. A client only looks at the entry whose version is
greater than its own `AppVersion`, and only at `stable` unless it is a beta
or alpha build.

## The trust chain

Update packages are v2 envelopes. They are authorized by the keys in
`Telegram/Resources/update/`, embedded into every build at compile time by
`Telegram/cmake/generate_update_keys.cmake`:

* `root-public.pem` — the pinned root Ed25519 public key;
* `manifest.min.json` — the key manifest, with the channel keys, their
  expiry dates and the AND-of-ORs authorization per channel;
* `manifest.sig` — the detached raw 64-byte Ed25519 signature of the exact
  manifest bytes, made by the root key.

This fork has its own key material: an Ed25519 root key that signs the
manifest and one Ed25519 channel key (`fork-stable-1`) that is authorized
by it for the `stable` channel. The manifest authorizes `stable` with a
single group holding that one key id, so a package needs exactly one valid
signature, and only from a key the manifest trusts.

`Packer` re-verifies the whole chain before and after packing (see
`LoadV2Keys` and `VerifyPackedV2`), and `out/Release/test_update_verify`
runs the focused verification tests, so a package that clients would reject
never leaves the runner.

## Publishing

`Update channel.` (`.github/workflows/update_channel.yml`) is the supported
publishing path. It accepts an `update-<version>` tag to publish, a
`test-update-<version>` tag for a non-publishing pack/build test, or a
manual dispatch with an explicit version. Publishing tags must point to a
commit already merged into `custom`; tags let publishing work without
changing this fork's `dev` default branch. The workflow rejects nonnumeric,
non-increasing, or not-yet-merged releases, builds a Release configuration
in the CentOS environment with
`DESKTOP_APP_SPECIAL_TARGET=linux` and `DESKTOP_APP_DISABLE_AUTOUPDATE=OFF`,
packs `Telegram` and `Updater` with the channel key, and commits the
package and the feed to `gh-pages`.

The channel key is not in the repository: it is the `UPDATE_CHANNEL_KEY`
secret, written to a temporary file on the ephemeral runner for the Packer,
then removed. Losing it means publishing a new manifest with a new key, committed
after. Losing it means publishing a new manifest with a new key, committed
together with its public key and root signature, which is a reviewed
change to this repository like any other trust material change.
