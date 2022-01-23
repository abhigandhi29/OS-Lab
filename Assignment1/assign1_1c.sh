for file in $(find $1 -type f -name "*")
do
    echo "${file##*.}"
    [[ "${file##*.}" == "$file" ]] && { mkdir -p Nil ; find "$1" -type f ! -name "*.*" -exec mv {} ./Nil \; ; } || { mkdir -p "${file##*.}" ; find "$1" -type f -name "*.${file##*.}" -exec mv {} "./${file##*.}" \; ; } 
done