# Tests

Built only with the kirigami flavor (Qt 6), because the QML tests need the Qt 6 QML runtime.

```bash
cmake -B build -G Ninja -DFLAVOR=kirigami -DBUILD_TESTING=ON
cmake --build build
QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure
```

- `cpp/` - Qt Test unit tests of C++ code without Bluetooth, for example `TypeConversion`.
- `qml/` - Qt Quick Test. `tst_ComponentsLoad.qml` compiles every component from the app resources;
  the other files test single components.

Coverage of the C++ code (gcov):

```bash
cmake -B build-cov -G Ninja -DFLAVOR=kirigami -DBUILD_TESTING=ON -DENABLE_COVERAGE=ON
cmake --build build-cov && ctest --test-dir build-cov
gcovr -r . build-cov --html-details coverage.html
```

QML code has no line coverage in open-source Qt.
