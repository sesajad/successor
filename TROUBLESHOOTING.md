# Running in rescue mode

Press `e` in the boot menu and append `init=/bin/init2` to the kernel command line.

# Improper kernel / boot loader configuration

```
docker run -v /boot:/boot --rm the-image sh -c 'pacman -S --noconfirm linux linux-firmware mkinitcpio && mkinitcpio -p linux && bootctl --path=/boot install'
```