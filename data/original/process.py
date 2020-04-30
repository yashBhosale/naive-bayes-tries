import pandas as pd
import nltk
import re
import string

emails = pd.read_csv('./emails.csv')
delchars = ''.join(c for c in map(chr, range(256)) if not c.isalnum())
print(emails.groupby('spam').count())
wnl = nltk.WordNetLemmatizer()
stop_words = set(nltk.corpus.stopwords.words('english'))
emails['filtered_text'] = emails['text'].map(lambda text: re.sub('[^A-Za-z0-9]+', ' ', text)) \
    .map(lambda text: re.sub("\d+", "thisisanumber", text)) \
    .map(lambda text: nltk.tokenize.word_tokenize(text)) \
    .map(lambda text: text[1:]) \
    .map(lambda tokens: [w.lower() for w in tokens if not w in stop_words]) \
    .map(lambda text: ' '.join(text)) \
    .map(lambda text: wnl.lemmatize(text))


emails[emails['spam'] == 1].iloc[:10].to_csv('../test/spam_test.csv', index=None, columns = ["filtered_text", "spam"])
emails[emails['spam'] == 1].iloc[12:].to_csv('../train/spam_train.csv', index=None, columns = ["filtered_text", "spam"])

emails[emails['spam'] == 0].iloc[:10].to_csv('../test/ham_test.csv', index=None, columns = ["filtered_text", "spam"])
emails[emails['spam'] == 0].iloc[12:].to_csv('../train/ham_train.csv', index=None, columns = ["filtered_text", "spam"])
print("done")