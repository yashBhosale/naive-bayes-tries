#include <string>
#define SPAM 1
#define HAM 0
#define DEBUG false

class TrieNode{
    public:
        TrieNode * children[26];
        //0 is ham, 1 is spam
        int freq [2];
        int numChildren;
        TrieNode();

};

class Trie{
    public:
        TrieNode * root;
        int size[2];
        int cardinality;
        Trie();
        void insertMultiple(std::string& str, int n, bool isSpam);
        void insertOne(std::string& str, bool isSpam);
        int search(std::string str, bool isSpam);
        void removeMany(std::string& str, int n, bool isSpam);
        void removeAll(std::string str, bool isSpam);
        void removeOne(std::string str, bool isSpam);
        void destroySuffix(TrieNode* root, std::string & str, int startIndex);
        void destroyAllNodes(TrieNode * root);
};
