rm /.freespace/$USER/testfsfile;
touch /.freespace/$USER/testfsfile;
fusermount -u /tmp/$USER/mountdir/;
make clean;
make;
./sfs /.freespace/$USER/testfsfile /tmp/$USER/mountdir;
