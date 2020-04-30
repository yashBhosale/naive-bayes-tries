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


class NaiveBayes{

    public:
        Trie *naiveBayesTrie;
        std::unordered_map<int, float> spamProbabilities, hamProbabilities;

        NaiveBayes(){
            naiveBayesTrie = new Trie();
        }
    
        /**
         * @brief computes the likelihood of a word w appearing given class c
         * 
         * @param wordCount number of times w occurs in c
         * @param totalCount number of words in c
         * @param cardinality number of unique words in c
         * @return float 
         */
        double conditionalProbabilityLaplaceSmoothing(double wordCount, double totalCount, double cardinality){
            double intermediate =  (wordCount + 1) / ( totalCount +  cardinality); 
            return std::log10(intermediate);
        }

        // TODO: put conditional probabilities for training words into a hashmap<int, float> for o(1) amortized lookup
        // there's probably a lot of words that are going to appear, like, once or twice so this is a good space-saver and it's still 
        // time-efficient as well.
        void train(std::vector<std::string> inputfiles){
            std::string line;
            int isSpam;
            std::vector<std::string> intermediate, words;
            std::string headers;

            for(std::string inputName : inputfiles){
                std::ifstream inputFile(inputName);
                
                // put the data into the trie.
                if(inputFile.is_open()){
                    // just in case we want the headers (also because it would break things)
                    getline(inputFile, headers);

                    while(getline(inputFile, line)){
                        boost::split(intermediate, line, boost::is_any_of(","));
                        isSpam = std::stoi(intermediate[1]);
                        boost::split(words, intermediate[0], boost::is_any_of(" "));
                        for(std::string word : words){
                                naiveBayesTrie->insertOne(word, isSpam);
                        }
                        words.clear();
                        intermediate.clear();
                    }
                }

                inputFile.close();
            }

            hamProbabilities[0] = conditionalProbabilityLaplaceSmoothing(0, naiveBayesTrie->size[HAM], naiveBayesTrie->cardinality);
            spamProbabilities[0] = conditionalProbabilityLaplaceSmoothing(0, naiveBayesTrie->size[SPAM], naiveBayesTrie->cardinality);

            printf("%f %f\n", hamProbabilities[0], spamProbabilities[0]);
            printf("finished importing data into the trie.\n");
            printf("\tnumber of spam words: %d\n", naiveBayesTrie->size[SPAM]);
            printf("\tnumber of ham words: %d\n", naiveBayesTrie->size[HAM]);

            populateProbTable(naiveBayesTrie->root);
            printf("calculated probabilities\nTraining finished.\n");
        }

        void populateProbTable(TrieNode* root){
            if (root == NULL)
                return;

            if(root->freq[HAM] > 0 && hamProbabilities.find(root->freq[HAM]) == hamProbabilities.end())
                hamProbabilities[root->freq[HAM]] = conditionalProbabilityLaplaceSmoothing(root->freq[HAM], naiveBayesTrie->size[HAM], naiveBayesTrie->cardinality);
            if(root->freq[SPAM] > 0 && spamProbabilities.find(root->freq[SPAM]) == spamProbabilities.end())
                spamProbabilities[root->freq[SPAM]] = conditionalProbabilityLaplaceSmoothing(root->freq[SPAM], naiveBayesTrie->size[SPAM], naiveBayesTrie->cardinality);

            for(int i = 0; i < 26; i++){
                if(root->children[i] != NULL)
                    populateProbTable(root->children[i]);
            }
        }

        std::vector<float> predict(std::string message){

            std::vector<std::string> words;
            boost::split(words, message,boost::is_any_of(" "));
            double probability[2];
            std::vector<float> returnProbs;
            
            //calculate priors
            probability[HAM] = std::log((double)naiveBayesTrie->size[HAM]/((double)naiveBayesTrie->size[HAM] + naiveBayesTrie->size[SPAM]));
            probability[SPAM] = std::log((double)naiveBayesTrie->size[SPAM]/((double)naiveBayesTrie->size[HAM] + naiveBayesTrie->size[SPAM]));

            //calculate conditional probabilities
            for(std::string word : words){
                probability[SPAM] = probability[SPAM] + spamProbabilities[naiveBayesTrie->search(word, SPAM)];
                probability[HAM] = probability[HAM] + hamProbabilities[naiveBayesTrie->search(word, HAM)];
                if(DEBUG){
                    std::cout << spamProbabilities[naiveBayesTrie->search(word, SPAM)] << " " <<  hamProbabilities[naiveBayesTrie->search(word, HAM)] << "\n";
                    printf("\n");
                }
            }   

            returnProbs.push_back(probability[HAM]);
            returnProbs.push_back(probability[SPAM]);
            if(DEBUG)
                std::cout << "Spam | Ham\n" << returnProbs[SPAM] << " " << returnProbs[HAM]<<"\n\n";
            else
                printf("%s", (returnProbs[HAM]>returnProbs[SPAM]) ? "Ham\n":"Spam\n");
            return returnProbs;    
        }

        void predictFiles(std::string inputName){
            std::ifstream inputFile(inputName);
            std::string headers;

            std::string line;
            std::vector<std::string> columns, words;

            if(inputFile.is_open()){
                getline(inputFile, headers);
            
                while(getline(inputFile, line)){
                    boost::split(columns, line, boost::is_any_of(","));
                    std::vector<float> test = predict(columns[0]);
                    
                }
            }
        }

};

void runTests(){
    Trie *t = new Trie();

    //it can insert one word and many words into spam
    std::string oneSpam = "onespam", manySpam ="manyspam", oneHam="oneham", manyHam="manyham";
    t->insertOne(oneSpam, SPAM);
    t->insertMultiple(manySpam, 3, SPAM);

    t->insertOne(oneHam, HAM);
    t->insertMultiple(manyHam, 3, HAM);

    // search singly inserted words
    if(t->search(oneSpam, SPAM)==1) 
        printf("found a piece of spam\n");
    else 
        printf("search onespam failed\n");
    
    if(t->search(oneHam,HAM)==1)
        printf("found a piece of ham\n");
    else 
        printf("search oneHam failed\n");

    // search multiply-inserted words (is that a word?)
    if(t->search(manySpam, SPAM)==3) 
        printf("found some spam\n");
    else 
        printf("search manyspam failed\n");
    
    if(t->search(manyHam, HAM) == 3)
        printf("found some Ham\n");
    else
        printf("search manyHam failed\n");
    

    //correctly check cardinality of the trie
    if(t->cardinality == 4)
        printf("cardinality check: correct\n");
    else
        printf("cardinality check failed, cardinality is %d instead of 4\n", t->cardinality);

    if(t->size[SPAM] == 4 && t->size[HAM] ==4)
         printf("numWords check: correct\n");
    else
        printf("numWords check failed, spam has %d words and ham has %d words a\n", t->size[SPAM], t->size[HAM]);


    // remove words
    t->removeOne(oneSpam, SPAM);
    t->removeOne(oneHam, HAM);

    if(t->search(oneHam, HAM) == 0 && t->search(oneSpam, SPAM) == 0)
        printf("successfully deleted singly-inserted words\n");

    t->removeOne(manySpam, SPAM);
    t->removeOne(manyHam, HAM);

    if(t->search(manyHam, HAM) == 2 && t->search(manySpam, SPAM) == 2){
        printf("successfully deleted one instance of many-inserted words\n");
        
    }
    else{
        printf("did not successfully delete one instance of a manyInsert word\n");
        printf("ham: %d, spam: %d\n", t->search(manyHam, HAM), t->search(manySpam, SPAM));
    }
    t->removeMany(manyHam, 2, HAM);
    t->removeMany(manySpam, 2, SPAM);

    if(t->search(manyHam, HAM) == 0 && t->search(manySpam, SPAM) == 0)
        printf("successfully deleted multiples instances of many-inserted words\n");
    else
        printf("did not successfully delete multiple instance of a manyInsert word\n");

    //correctly check cardinality of the trie
    if(t->cardinality == 0)
        printf("cardinality check: correct\n");
    else
        printf("cardinality check failed, cardinality is %d instead of 0\n", t->cardinality);

    if(t->size[SPAM] == 0 && t->size[HAM] == 0)
         printf("numWords check: correct\n");
    else
        printf("numWords check failed, spam has %d words and ham has %d words a\n", t->size[SPAM], t->size[HAM]);

    delete t->root;
    delete t;
    
}



int main(int argc, char ** argv){
    runTests();
    printf("\n");
    NaiveBayes *nb = new NaiveBayes();
    std::vector<std::string> inputFiles;

    inputFiles.push_back("data/train/spam_train.csv");
    inputFiles.push_back("data/train/ham_train.csv");

    nb->train(inputFiles);
    nb->predictFiles("./data/test/spam_test.csv");
}