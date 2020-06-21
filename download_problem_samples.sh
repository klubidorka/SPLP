if ! [ -d /samples/ ]; then
mkdir samples
fi
cd samples
wget -e robots=off -q -r -np -nd -A 'cap[0-9][0-9].txt' http://people.brunel.ac.uk/~mastjjb/jeb/orlib/files/
wget -e robots=off -q -r -np -nd -A 'cap[0-9][0-9][0-9].txt' http://people.brunel.ac.uk/~mastjjb/jeb/orlib/files/
wget -q "http://people.brunel.ac.uk/~mastjjb/jeb/orlib/files/uncapopt.txt"
echo "Download complete"
