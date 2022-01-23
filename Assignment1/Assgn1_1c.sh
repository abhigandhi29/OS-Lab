extensions=($(find $1 -type f -name "*.*" | awk -F. '{print $NF}' | sort -u))
for extension in ${extensions[@]}; do
    echo $extension
    mkdir -p $extension 
    files=($(find $1 -type f -name "*.$extension"))
    for ((i=0; i<${#files[*]}; i+=1000)); do
        mv ${files[@]:i:1000} $extension
    done  
done
mkdir -p Nil 
files=($(find $1 -type f))
for ((i=0; i<${#files[*]}; i+=1000)); do
    mv ${files[@]:i:1000} Nil
done  
