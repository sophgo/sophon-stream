# sophon-stream qt_display element

English | [简体中文](README.md)

Sophon-stream `qt_display` element is a plugin within the Sophon-stream framework. It serves as a specialized tool for QT display.

## 1. Configuration Parameters
Sophon-stream qt_display plugin comes with several configurable parameters that can be adjusted according to requirements. Here are some commonly used parameters:

```json
{
  "configure": {
      "width": 1920,
      "height": 1080,
      "rows": 2,
      "cols": 3
  },
  "shared_object": "../../build/lib/libqt_display.so",
  "name": "qt_display",
  "side": "sophgo",
  "thread_number": 4
}
```

| Parameter Name       | type   | Default value                                | Description                   |
| ------------- | ------ | ------------------------------------ | ------------------------------- |
| width         | int    | 1920                                  | screen width            |
| height        | int    | 1080                                 | screen height            |
| rows          | int    | 2                                    | number of stream on each rows  |
| cols          | int    | 3                                    | number of stream on each cols   |
| shared_object | string | "../../../build/lib/libqt_display.so" | libqt_display dynamic library path |
| name          | string | "qt_display"                          | element name                     |
| side          | string | "sophgo"                             | device type                      |
| thread_number | int    | 4                                    | thread num                      |


> **notes**
1. When using the `qt_display` element, it's important to ensure that the number of qt stream is greater than or equal to the number of input stream routes.
2. thread_number should be the same as total number of input stream routes.