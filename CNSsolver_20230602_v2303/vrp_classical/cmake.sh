cd ./code/pybind11
if [ ! -d build ];then
	mkdir build
fi
cd build/
if [ -e CMakeCache.txt ];then 
	rm -f CMakeCache.txt
fi

cmake -DCMAKE_BUILD_TYPE=Release ..
if [ $? -gt 0 ];then
	echo ===============
	echo Error Occured
	exit 1
fi
cmake --build .
if [ $? -gt 0 ];then
	echo ===============
	echo Error Occured
	exit 1
fi

cp -f ./*.so ../../

echo ===============
echo Done.

exit 0
