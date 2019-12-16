## Grid server

Download `selenium-server-standalone-X.Y.Z.jar` from [SeleniumHQ] and start:

    $ java -jar selenium-server-standalone-2.44.0.jar -role hub

It shoudl print out stg like this:

    07:47:03.155 INFO [Hub.start] - Nodes should register to http://10.223.161.7:4444/grid/register/
    ^^^^ this URL have to be passed to Node (next step)



## Node on Windows with Edge

Download and run VM with evaluation version of Windows with Edge: https://developer.microsoft.com/en-us/microsoft-edge/tools/vms/

Download Microsoft WebDriver: https://developer.microsoft.com/en-us/microsoft-edge/tools/webdriver/

(You should have `MicrosoftWebDriver.exe` most probably in your `Downloads` folder)

Download `selenium-server-standalone-X.Y.Z.jar` from [SeleniumHQ].

Start the node (update `-hub` URL and paths):

    "C:\Program Files\Java\jre-10.0.2\bin\java" \vm
        -Dwebdriver.edge.driver="C:\Users\IEUser\Downloads\MicrosoftWebDriver.exe" \
        -jar C:\Users\IEUser\Downloads\selenium-server-standalone-3.14.0.jar \
        -role node -hub http://10.1.3.65:4444/grid/register \
        -browser "browserName=MicrosoftEdge, platform=WINDOWS, maxInstances=10"

Copy-paste compatible

    "C:\Program Files\Java\jre-10.0.2\bin\java" -Dwebdriver.edge.driver="C:\Users\IEUser\Downloads\MicrosoftWebDriver.exe" -jar C:\Users\IEUser\Downloads\selenium-server-standalone-3.14.0.jar -role node -hub http://10.1.3.65:4444/grid/register  -browser "browserName=MicrosoftEdge, platform=WINDOWS, maxInstances=10"

    "/cygdrive/c/Program Files/Java/jre-10.0.2/bin/java" -Dwebdriver.edge.driver="C:/Users/IEUser/Downloads/MicrosoftWebDriver.exe" -jar C:/Users/IEUser/Downloads/selenium-server-standalone-3.14.0.jar -role node -hub http://10.1.3.65:4444/grid/register  -browser "browserName=MicrosoftEdge, platform=WINDOWS, maxInstances=10"


Run test against Selenium Grid HUB:

    SELENIUM_REMOTE_URL=http://10.1.3.65:4444/wd/hub \
        SELENIUM_BROWSER=MicrosoftEdge \
        npx ts-node ./test/some-selenium-test.ts

[SeleniumHQ]: https://www.seleniumhq.org/download/

###

# Managing software on machine:

https://chocolatey.org/install#installing-chocolatey

# Running selenium on Windows

## Install

* Chocolatey:
```
@"%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe" -NoProfile -InputFormat None -ExecutionPolicy Bypass -Command "iex ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))" && SET "PATH=%PATH%;%ALLUSERSPROFILE%\chocolatey\bin"
```

* Browsers and their drivers
```
choco install javaruntime firefox selenium-gecko-driver googlechrome selenium-chrome-driver selenium-ie-driver selenium-edge-driver
```

## Run
Run this:

* as a standalone node

```
choco install -y selenium --params "'/port:8888 /log'"
choco install -y --force selenium --params "'/port:8888 /log /autostart'"
```

* connected to HUB:
```
choco install -y selenium --params "'/port:8888 /log /hub:http://localhost:4444 /autostart''"
choco install -y --force selenium --params "'/port:8888 /log /hub:http://localhost:4444 /autostart'"
```

Reference: https://github.com/dhoer/choco-selenium
## Automating stuff on Windows

### Access

`vagrant ssh` more or less works but is shit.
One can `cmd`, so you have proper windows environment but without any terminal features.

