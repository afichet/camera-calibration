find lib -regex '.*\.\(c\|h\)' -exec clang-format -style=file -i {} \;
find apps -regex '.*\.\(c\|h\)' -exec clang-format -style=file -i {} \;