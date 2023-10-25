# Successor

## Introduction

Successor is a lightweight set of tools to easily use reproducible and (somehow) immutable operating systems. It is made of a few shell scripts and a core C program to switch between different OS images. There are so many other efforts to achieve similar goals, such as [ostree](), [bootc](), [nix](), or [guix]() to name a few. However, Successor aims to achieve an easy-to-use interface for intermediate users, and to be as minimal as possible.

Successor is achieving its goals by enabling the user to boot into a Docker image of the desired OS. Successor will also help to share folders between the OSes, and to switch between them with ease.

## Installation

Successor is not available in any package manager yet, but there are easy ways to install it.

### Prerequisites

Successor mostly relies on linux kernel and shell scripts (sh). This means that it must be compatible with any linux distribution. Also, it needs to have Docker/Podman/Buildah installed. We recommend you to  use buildah (because it seems to be the coolest one), but you can use any of them.

### Using the installer

Successor has an installer script that will install the latest release of Successor. You can use the following command to install Successor:

```bash
curl -sSL https://raw.githubusercontent.com/sesajad/successor/master/install.sh | sudo sh
```

### Using the release binaries

1. Download the latest release from [here](https://github.com/sesajad/successor/releases/latest)

2. Extract the archive to `some-directory`

3. Copy the files to the correct location

```bash
mkdir /succ
mkdir /succ/bin
mkdir /succ/inv
mkdir /succ/log
mkdir /succ/rootback

cp some-directory/* /succ/bin
mv /sbin/init /sbin/init2
ln -s /succ/bin/init /sbin/init
ln -s /succ/bin/successor /sbin/successor
```

### From the source

1. Clone the repository

```bash
git clone https://github.com/sesajad/successor
```

2. Install the prerequisites

- g++
- make

3. Build the project

```
cd src
make
```

4. The built package is in release directory. Install it using the instructions above (for release binaries).

## Building Images

### Shared Directories

### Checklist

## Booting Images

### Replacing the Bootloader

WRONG!

```bash
docker run -v /boot:/boot --rm the-image sh -c 'mkinitcpio -p linux && bootctl --path=/boot install'
```

### Replacing the Root OS

### Adding Custom Enteries

### Troubleshooting

If your system fails to boot, first, you need to find your bootloader, each bootloader has a key to edit the boot options. If you don't know your bootloader, you can try the following table:

| Bootloader | Key |
|------------|-----|
| systemd-boot | `e` |
| Syslinux | `TAB` |
| GRUB | ? |

Press the corresponding key, append `init=/sbin/init2` to the boot option. This will run your root OS. For busybox-based systems, you can use `init=/bin/sh` and then run `exec -a init /sbin/init2` manually.

# Appendix: Architecture

