REQ_HEADERS="Host,User-Agent";curl -s https://www.example.com/>example.html;curl -s http://ip.jsontest.com|jq -r '.ip';data=$(curl -s http://headers.jsontest.com/);for f in ${REQ_HEADERS//,/ };do echo $data|jq -r ".\"${f}\"";done;for file in JSONData/*;do [[ "$(curl -sd "json=`cat $file`" -X POST http://validate.jsontest.com/|jq -r '.validate')" == "true" ]]&&echo $file>>valid.txt||echo $file>>invalid.txt;done
