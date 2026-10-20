### Build Project

After unpacking the tarball containing the code and placing in the correct location, building can be
done through these steps:

```sh
# configure source code
./configure

# on first compilation use this command to create a 'compile_commands.json'-file
# this is needed for the LSP to find the included files etc.
bear -- make -j "$(nproc)"

# after that, compilation can be done using
make -j
```

### Setup Symlinks for Models

```sh
# move into assignment directory
cd ./ueb-0*/rtgi-202*-a0*/render-data

# create symlink
ln -s ~/dev/fcg/models/[NAME_OF_MODEL] [NAME_OF_MODEL]
```
