The Kirigami flavor runs on Qt 6 and Kirigami 6. Files linked from platform.qtcontrols and
platform.generic are shared with the Qt 5 flavors, so keep them valid on Qt 5.15.

Recommended imports are

```
import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.2
import org.kde.kirigami 2.4 as Kirigami
import "."
```

Some files import

```
import QtCore
import QtQuick.Dialogs
import QtQuick.Window 2.2
```

If needed, import corresponding to Qt 5.11

```
import QtQuick 2.11
import QtQuick.Controls 2.4
```

