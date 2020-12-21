# Build

## Ubuntu Touch

[Clickable](https://clickable-ut.dev/en/latest/install.html) is used to build and
debug the app for Ubuntu Touch.

Download dependencies:

```bash
clickable prepare-deps
```

Build dependencies:

```bash
clickable build-libs --arch arm64 # or amd64 or armhf
```

Build the app:

```bash
clickable build --arch arm64 # or amd64 or armhf
```

For more details see [Clickable Documentation](https://clickable-ut.dev/en/latest/usage.html).
