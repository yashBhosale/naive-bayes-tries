#include <string>
#include <stack>
#include <ios>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <cmath>
#include "Trie.h"

#define SPAM 1
#define HAM 0
#define DEBUG false

        

    TrieNode::TrieNode(){
        for(int i = 0; i < 26; i++)
            children[i] = NULL;

        for(int i = 0; i < 2; i++) 
            freq[i] = 0;
        numChildren = 0;
    }



        Trie::Trie(){
            this->root = new TrieNode();
            this->size[0] = 0;
            this->size[1] = 0;
            this->cardinality = 0;
        }

        /**
         * Insert a string, telling the trie which message you're inserting from (this is bayes filter-specific)
         *
         * @param str the string you're inserting (duh)
         * @body body the number of the body that this word is being inserted from (bayes-specific)
         */
        void Trie::insertMultiple(std::string& str, int n, bool isSpam){
            TrieNode * temp = this->root;
            int length = str.length();

            for(int i = 0; i < length; i++){
                //if the child doesn't exist (this suffix has not been inserted yet) create the child)
                if(temp->children[str[i] - 'a'] == NULL){
                    temp->numChildren++;
                    temp->children[str[i] - 'a'] = new TrieNode();
                }
                
                //iterate to the child    
                temp = temp->children[str[i] - 'a'];
            }

            if(temp->freq[isSpam] == 0 || temp->freq[!isSpam] == 0)
                this->cardinality +=1;
            temp->freq[isSpam] += n;
            this->size[isSpam] += n;
            return;
        }

        void Trie::insertOne(std::string& str, bool isSpam){
            this->insertMultiple(str, 1, isSpam);
        }

        /**
         * Search for whether a body exists in the corpus
         * 
         * Note: if you want to tell whether a body exists in a specific corpus, 
         * you'll have to refactor the code to allow that ;)
         * 
         * @param str the string that you want to check the existence of
         */
        int Trie::search(std::string str, bool isSpam){
            TrieNode * temp = this->root;
            int length = str.length();

            for(int i = 0; i < length; i++){
                if(temp->children[str[i]-'a'] == NULL)
                    return 0;
                temp = temp->children[str[i]-'a'];
            }

            return temp->freq[isSpam];   
        }

        /**
         * Remove n instances of the word. Aka, reduce its frequency by 1. 
         * If its frequency becomes less than or equal to zero,
         * free all the nodes from the last fork to the end of the useless suffix.
         * 
         * @param str the string you want to remove
         * @param n the number of instances you want to remove
         */
        void Trie::removeMany(std::string& str, int n, bool isSpam){
            TrieNode * temp = this->root;
            TrieNode * safePrefix = this->root;
            int suffixIndex = 0;
            int length = str.length();

            //iterate to the end of the string
            for(int i = 0; i < length; i++){
                // implies that the word wasn't inserted in the first place
                if(temp->children[str[i]-'a'] == NULL)
                    return;

                // if this prefix is a prefix for another inserted word OR is its own word
                // then set firstToDie to equal the first value in its suffix 
                if (i < length - 1 && (temp->numChildren > 1 || temp->freq[isSpam] > 0 || temp->freq[!isSpam] > 0 ) ){
                    safePrefix = temp;
                    suffixIndex = i;
                }
                
                temp = temp->children[str[i]-'a'];
            }

            //string itself wasn't inserted as a word, it is a substring of some other word
            if(temp->freq[isSpam] == 0)
                return;

            if(n == -1){
                this->size[isSpam] -= temp->freq[isSpam];
                temp->freq[isSpam] = 0;
            }
            else{
                this->size[isSpam] -= n;
                temp->freq[isSpam]-= n;
            }

            //if the current string is not a prefix and the frequency of current string is 0 after removing 1 
            if(temp->freq[isSpam] <= 0 && temp->freq[!isSpam] <= 0){
                this->cardinality-=1;
                if(temp->numChildren == 0){
                    destroySuffix(safePrefix, str, suffixIndex);
                    safePrefix->children[str[suffixIndex] - 'a'] = NULL;
                    safePrefix->numChildren--;
                }
            }

            return;
        }

        void Trie::removeAll(std::string str, bool isSpam){
            this->removeMany(str, -1, isSpam);
        }

        void Trie::removeOne(std::string str, bool isSpam){
            this->removeMany(str, 1, isSpam);
        }


        /**
         * Deletes an unnecessary suffix in O(n) time and O(1) space, where n is the length of the suffix
         * 
         * @param root the last node in the prefix
         * @param str the string that contains the suffix
         * @param startIndex index of the first character, in the string, of the suffix to delete
         */ 
        void Trie::destroySuffix(TrieNode* root, std::string & str, int startIndex){
            int length = str.length();
            TrieNode* container = root->children[str[startIndex] - 'a'];
            TrieNode* head;

            for(int i = startIndex+1; i < length; i++){
                head = container->children[str[i]-'a'];
                container->children[str[i]-'a'] = NULL;
                delete container;
                container = head;
            }
             delete container;
        }

        /**
         * @brief Delete the entire trie starting from this root using backtracking, no memory leaks;
         * 
         * @param root the root of the trie you want to destroy 
         */
        void Trie::destroyAllNodes(TrieNode * root){
            if(root == NULL)
                return;
            for(int i = 0; i < 26; i++)
                destroyAllNodes(root->children[i]);
            delete root;
            return;
        }




