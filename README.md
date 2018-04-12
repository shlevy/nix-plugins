nix-plugins
============

A collection of useful [Nix] native plugins.

[Nix]: https://nixos.org/nix

extra-builtins
----------------

This plugin adds a setting `extra-builtins-file` and two builtins:

* `extraBuiltins`: If the `extra-builtins-file` exists, it is imported
  and passed a set containing at least the `importNative` and `exec`
  primops, even if `allow-unsafe-native-code-during-evaluation` is
  `false`, and the result is available as `extraBuiltins`. If the
  file does not exist, `extraBuiltins` will be `null`.
* `nix-cflags`: A set of required flags needed to build a native
  plugin against the same version of Nix this plugin is compiled
  against. Includes linker flags as well, but is named `nix-cflags`
  for compatibility. See [nix-plugins-config.h.in][config_h_in] for
  the list of attributes in the set.

This allows users to specify a fixed set of safe extra builtins
without enabling arbitrary Nix expressions to run arbitrary native
code. The expectation is that `extra-builtins-file` defines a set of
builtins, but ultimately that's up to the end user.

[config_h_in]: ./nix-plugins-config.h.in
