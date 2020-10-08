find lib -regex '.*\.\(c\|cpp\|h\)' -exec clang-format -style=file -i {} \;
find apps -regex '.*\.\(c\|cpp\|h\)' -exec clang-format -style=file -i {} \;
