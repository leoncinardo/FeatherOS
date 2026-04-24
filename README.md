
# FeatherOS
A WIP x86_64 hobby operating system made in C and Assembly.


## 1. But why this project?
The real question is why not? I like challenges so why not create an operating system? I want to have a deeper insight on how computers work and attempt to create projects to sharpen my skills and abilities.

<!-- ## 2. Compiling
Before trying to compile you need these:
 - a GCC cross compiler for x86_64
 - NASM installed


### 2.1 Generating OS images
The makefile can generate .iso and .hdd images but first you need to get all needed dependencies for the kernel(for now they are 3) and the bootloader(Limine), so you need an internet connection. To do so open you're favorite terminal and cd into the `kernel/` directory and run:

```sh
make getDeps
```

After this cd back into the main project directory.
To generate a .iso image simply run:
```sh
make
```

To generate a .hdd image run:
```sh
make all-hdd
```

Both commands will download the `Limine bootloader` the first time they're run.

### 2.2 Compiling only the kernel
You first need to download needed dependecies as shown in section 2.1.
From the main project directory run:
```sh
make kernel
```

or from the `kernel/` directory run:
```sh
make
``` -->