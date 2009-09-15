#ifndef _str_tree
#define _str_tree

typedef struct str_tree {
  char *s;
  void *extra;
  struct str_tree *left, *right;
} str_tree;

str_tree *str_tree_add(str_tree **root, const char *s, void *extra);
str_tree *str_tree_find(str_tree *node, const char *s);
void str_tree_free(str_tree **root);
int str_tree_traverse(str_tree *root,int(*f)(str_tree *node));

#endif /* _str_tree */
