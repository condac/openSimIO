
int findNextChar( char str[], int cur, char sep) {
  int res = cur;
  while (str[res] != '\0') {
    if (str[res] == sep) {
      return res;
    }
    res++;
  }
  return -1;
}

int getNextSubStr(char str[], char substr[], int current, char sep) {

  int next = findNextChar(str, current, sep);
  if (next != -1) {
    strncpy(substr, str+current, next-current);
    substr[next-current] = '\0';
  
    current = next+1;
    return current;
  } else {
    return -1;
  }
  
}
