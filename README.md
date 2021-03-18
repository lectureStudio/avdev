## Extended audio/video device support for Java

Java native interface implementation for audio/video devices. The goal of this project is to add additional functionality to audio/video devices that Java is missing. For example, this would be the retrieval of the default audio playback and capture device set by the system, the notification of audio volume changes and the notification when a audio or video device has been connected or disconnected.

```xml
<dependency>
    <groupId>org.lecturestudio.avdev</groupId>
    <artifactId>avdev</artifactId>
    <version>0.1.0-SNAPSHOT</version>
</dependency>
```

### Supported Platforms
By default, the main artifact depends on the native library corresponding to the system you are running your build or application on.
The native libraries can be loaded on the following platforms:

| Operating System | Classifier          |
| ---------------- |---------------------|
| Linux            | linux-x86_64        |
| macOS            | macos-x86_64        |
| Windows          | windows-x86_64      |

### Build Notes

In order to build the native code, be sure to install the prerequisite software and libraries:

<table>
  <tr>
    <td>Linux</td>
    <td>gcc, g++, libstdc++, libpulse-dev, libudev-dev, libjpeg62-turbo-dev, libv4l-dev (e.g. for Debian, names may differ depending on your distro)</td>
  </tr>
  <tr>
    <td>macOS</td>
    <td>Xcode 9 or higher</td>
  </tr>
  <tr>
    <td>Windows</td>
    <td>Visual Studio 2017 or higher</td>
  </tr>
</table>

Assuming you have all the prerequisites installed for your OS, run:

```
mvn install
```
