# Budgie Trash Applet
Manage items in your trash bin right from the Budgie panel.

![Main View](https://i.imgur.com/riBmriJ.png) ![Confirm Delete All](https://i.imgur.com/DW7ZlRj.png) ![Confirm Restore](https://i.imgur.com/QZkqu07.png)

---

## Development

### Dependencies
```
budgie-1.0 >= 2
gtk+-3.0 >= 3.22
glib-2.0 >= 2.46.0
vala
```
You can get these on Solus with the following packages:
```
budgie-desktop-devel
libgtk-3-devel
glib2-devel
vala
```

### Building and Installing
1. Configure the build directory and meson with:
```bash
mkdir build
meson --prefix=/usr build
```

2. Build the project with:
```bash
ninja -C build
```

3. Install the files with:
```bash
sudo ninja install -C build
```
