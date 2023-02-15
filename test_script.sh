for file in tests/*
do
  if ./grader engine < "$file" 2>&1 | grep -o 'test passed' | grep -q 'test passed'; then
    echo "$file passed"
  else
    echo "$file failed"
  fi
done