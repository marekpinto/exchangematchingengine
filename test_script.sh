for file in tests/*
do
  if ./grader engine < "$file" | grep -q "test passed"; then
    echo "$file passed"
  else
    echo "$file failed"
  fi
done
