# Successor

Successor is a lightweight set of tools to easily use reproducible and (somehow) immutable operating systems.
The whole idea of Successor is to use Docker (or any other OCI-compatible software) to build an OS image and to boot into it while keeping the current OS as a fallback.

## Installation

### Using the installer

```bash
curl -sSL https://raw.githubusercontent.com/sesajad/successor/master/install.sh | sudo sh
```

For other ways of installation check out [here]

## Usage

**CAUTION: Successor is a relatively safe tool, it is unlikely to remove a file but it is possible to easily make your system unbootable.**

Successor moderates the boot process so that the latest OS built by successor is booted. If there is no such OS, your current OS (that we call it the root OS) will be booted. Successor OSes are stored in `/succ/inv` directory, and can be managed using `build`, `list` and `remove` commands.

In order to build a new OS, You need to create a Dockerfile/Containerfile. You can use the [example](https://github.com/sesajad/successor/blob/master/example/Containerfile) as a base. We highly recommend you to see [tips and tricks](https://github.com/sesajad/successor/blob/master/MANUAL.md#Tips) to make your next OS more usable.

Then, run the following command in the place that your `Containerfile` is located:

```bash
successor build -n some-cool-name ./Containerfile
```

Now, you can test your new OS with

```bash
successor run -n some-cool-name -e /bin/sh
```
If your next OS needs a relatively different kernel and initramfs (than your current one), like in the case that you are switching from alpine (that uses OpenRC) to Ubuntu (that uses systemd), you need to update the bootloader configuration. (see [here](https://github.com/sesajad/successor/blob/master/MANUAL.md#Bootloader))

Successor shares the home directory of the root OS with all of the OSes. If you want to share other directories, see [here](https://github.com/sesajad/successor/blob/master/MANUAL.md#Mounting)

Now, you can reboot to your new OS and see if it is running using the following command:

```bash
successor list
```

It is over, you can now even [replace your root OS](https://github.com/sesajad/successor/blob/master/MANUAL.md#Removing) (with caution) or do so many amazing things. To learn more check out the [manual](https://github.com/sesajad/successor/blob/master/MANUAL.md).

## Contributing

Just like any other open source project, Successor is open to contributions.