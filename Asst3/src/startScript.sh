mkdir /.freespace/$USER;
mkdir /tmp/$USER;
#rm /.freespace/$USER/testfsfile;
touch /.freespace/$USER/testfsfile;
fusermount -u /tmp/$USER/mountdir/;
make clean;
make;
./sfs -s /.freespace/$USER/testfsfile /tmp/$USER/mountdir;

