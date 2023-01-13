cd ..
cd syscalls/
echo "Creating executable"
read a
make $a.exe
make $a
cd ..
cp syscalls/to_fsdir/$a fsdir/
./createfs -i fsdir -o student-distrib/filesys_img
cd ..
cd student-distrib/