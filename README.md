# Successor

Successor is a lightweight tool to easily use reproducible and (somehow) immutable operating systems.
The whole idea of Successor is to use Docker (or any other OCI-compatible software) to build an OS image and to replace the current OS with the new one easily.

## Installation

### Using the installer

```bash
curl -sSL https://raw.githubusercontent.com/sesajad/successor/master/installer.sh | sudo sh
```

### Manual installation

1. Download the project.

```
git clone git@github.com:sesajad/successor.git
```
2. (Optional) build the project.

```
cd src
make build
```

2. Copy the `succ` directory to `/succ`.

```
sudo cp -r succ /succ
```

3. Modify your kernel parameters with init=/succ/init. Depending on your bootloader, you may need to use a different method. See [this](https://wiki.archlinux.org/index.php/Kernel_parameters) for more information.

## Usage

**CAUTION: Successor can easily distroy your files, use it at your own risk.**

1. Make your new OS

You need to create a Dockerfile/Containerfile for your OS. You can use the [example](https://github.com/sesajad/successor/blob/master/example/Containerfile) as a base. We highly recommend you to see [tips and tricks](https://github.com/sesajad/successor/blob/master/TIPS.md) to make your OS more usable.

To build your OS, you need to set your image output to `/succ/new/`. You can do it with the following command:

```bash
mkdir -p /succ/new/
docker build -t the-image . --output type=local,dest=/succ/new/
```

Scripts for Docker and Podman are provided in `/succ/example/`.

2. Specify migration directories

Now, you must select your persistent and migrating directories in `/succ/directories.yml`. The example must work for most of the users.

3. Optional: Modify the mount script

You can modify the mount script in `/succ/mount.sh` to mount your root partition in `/succ/old/`. You should change it if your root partition is not `/dev/sda1`.

3. Dry-run the migration

You can dry-run the migration with the following command:

```bash
sudo /succ/init -t
```

Finally, you can reboot your system and you will be using your new OS.