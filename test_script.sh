#for file in tests/*
#do
#  if ( ./grader.sh engine < "$file" ) | grep -o 'test passed' | grep -q 'test passed'; then
#    echo "$file passed"
#  else
#    echo "$file failed"
#  fi
#done
for file in tests/mediumDir/*
do
    ./grader engine < "$file" | grep 'test passed';
done
