# Successor

## Introduction

Successor is a lightweight set of tools to easily use reproducible and (somehow) immutable operating systems. It is made of a few shell scripts and a core C program to switch between different OS images. There are so many other efforts to achieve similar goals, such as [ostree](), [bootc](), [nix](), or [guix]() to name a few. However, Successor aims to achieve an easy-to-use interface for intermediate users, and to be as minimal as possible.

Successor is achieving its goals by enabling the user to boot into a Docker image of the desired OS. Successor will also help to share folders between the root OS and the Docker image, and to switch between different images.

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

### From the source

## Building Images

### Shared Directories

### Checklist

## Booting Images

### Replacing the Bootloader

WRONG!

```bash
docker run -v /boot:/boot --rm the-image sh -c 'pacman -S --noconfirm linux linux-firmware mkinitcpio && mkinitcpio -p linux && bootctl --path=/boot install'
```

### Replacing the Root OS

### Adding Custom Enteries

### Troubleshooting

If your system fails to boot, first, you need to find your bootloader, each bootloader has a key to edit the boot options. 

| Bootloader | Key |
|------------|-----|
| systemd-boot | `e` |
| Syslinux | `TAB` |
| GRUB | `e` |

After you pressed the key, append `init=/sbin/init2` to the kernel command line. This will run your root OS. If you need to run the successor images, you can use `init=/succ/images/{REVISION_NUMBER}/sbin/init` instead.


# Appendix: Architecture

