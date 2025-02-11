# Bash Debugger
Debugger is a lightweight and efficient debugging tool designed for Bash scripts. It helps developers trace script execution, identify errors, and gain insights into script behavior.
Debugger creates a structured tree of HTML files for easier log analysis of larger scripts.
## Instalation
```
git clone
cd Debugger
make
```
Note: To make `debugger` work properly and be accessible from anywhere, add its location to your `$PATH` variable.

### Install via Package manager (TODO)

## Usage

```
debugger
```
This will create a root `index.html` in the default directory.

### Basic Usage
```
debugger script.sh
```
This will run `script.sh` with a debugger and create missing `index.html` files

**Currently two main features are supported:**

> [!IMPORTANT]
> `$DEBUG` and `$CALLHANDLER` environment variables connot be used. They are essential for supported features to work

### $DEBUG tag

Users can use the `$DEBUG` tags inside their scripts. It provides information to the `debugger` on where to create a new node in the HTML tree. The `debugger` will colapse `script_with_debug.sh` logs into separete file and link them to the parent. `$DEBUG` tags does not change the behavior of the script.
```
$DEBUG script_with_debug.sh
```

### $CALLHANDLER tag

The `$CALLHANDLER` tag can be used before `nohup` scripts to allow debugging of programs running in the background. `$CALLHANDLER` tags does not change the behavior of the script.
```
$CALLHANDLER bash -c "nohup $DEBUG script.sh </dev/null &>/dev/null &"
```

## CONFIGURATION
You can configure the Debugger using environment variables.
Currently supported variables are:
 - `$DEFAULT_PATH` -  The default path where the `/debugger_log` directory with the root index.html will be created.
