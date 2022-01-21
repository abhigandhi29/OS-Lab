echo $1
for file in $(find $1 -type f -name "*")
do
    extension="${file##*.}"
    echo $file
    echo $extension
    if [ "$extension" = "$file" ]; then
        mkdir -p Nil 
        find "$1" -type f ! -name "*.*" -exec mv {} ./Nil \;
    else
        mkdir -p $extension 
        find "$1" -type f -name "*.$extension" -exec mv {} "./$extension" \;
    fi    
done