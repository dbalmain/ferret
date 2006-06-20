#include "index.h"
#include "test.h"
#include "testhelper.h"

static char *body = "body";
static char *title = "title";
static char *text = "text";
static char *author = "author";
static char *year = "year";
static char *changing_field = "changing_field";
static char *tag = "tag";

extern HashTable *dw_invert_field(DocWriter *dw,
                                  FieldInverter *fld_inv,
                                  DocField *df);
extern FieldInverter *dw_get_fld_inv(DocWriter *dw, const char *fld_name);
extern void dw_reset_postings(HashTable *postings);

static FieldInfos *prep_all_fis()
{
    FieldInfos *fis = fis_new(0, INDEX_YES, TERM_VECTOR_NO);
    fis_add_field(fis, fi_new("tv", 0, INDEX_YES, TERM_VECTOR_YES));
    fis_add_field(fis, fi_new("tv un-t", 0, INDEX_UNTOKENIZED,
                              TERM_VECTOR_YES));
    fis_add_field(fis, fi_new("tv+offsets", 0, INDEX_YES,
                              TERM_VECTOR_WITH_OFFSETS));
    fis_add_field(fis, fi_new("tv+offsets un-t", 0, INDEX_UNTOKENIZED,
                              TERM_VECTOR_WITH_OFFSETS));
    return fis;

}

static void destroy_docs(Document **docs, int len)
{
  int i;
  for (i = 0; i < len; i++) {
    doc_destroy(docs[i]);
  }
  free(docs);
}

/*
static FieldInfos *prep_book_fis()
{
    FieldInfos *fis = fis_new(STORE_YES, INDEX_YES,
                              TERM_VECTOR_WITH_POSITIONS_OFFSETS);
    fis_add_field(fis, fi_new("date", STORE_YES, INDEX_NO, TERM_VECTOR_NO));
    return fis;

}

#define BOOK_LIST_LENGTH 37
Document **prep_book_list()
{
    Document **docs = ALLOC_N(Document *, BOOK_LIST_LENGTH);
    docs[0] = doc_new();
    doc_add_field(docs[0], df_add_data(df_new(author), 
			estrdup("P.H. Newby")))->destroy_data = true;
    doc_add_field(docs[0], df_add_data(df_new(title), 
			estrdup("Something To Answer For")))->destroy_data = true;
    doc_add_field(docs[0], df_add_data(df_new(year), 
			estrdup("1969")))->destroy_data = true;
    docs[1] = doc_new();
    doc_add_field(docs[1], df_add_data(df_new(author), 
			estrdup("Bernice Rubens")))->destroy_data = true;
    doc_add_field(docs[1], df_add_data(df_new(title), 
			estrdup("The Elected Member")))->destroy_data = true;
    doc_add_field(docs[1], df_add_data(df_new(year), 
			estrdup("1970")))->destroy_data = true;
    docs[2] = doc_new();
    doc_add_field(docs[2], df_add_data(df_new(author), 
			estrdup("V. S. Naipaul")))->destroy_data = true;
    doc_add_field(docs[2], df_add_data(df_new(title), 
			estrdup("In a Free State")))->destroy_data = true;
    doc_add_field(docs[2], df_add_data(df_new(year), 
			estrdup("1971")))->destroy_data = true;
    docs[3] = doc_new();
    doc_add_field(docs[3], df_add_data(df_new(author), 
			estrdup("John Berger")))->destroy_data = true;
    doc_add_field(docs[3], df_add_data(df_new(title), 
			estrdup("G")))->destroy_data = true;
    doc_add_field(docs[3], df_add_data(df_new(year), 
			estrdup("1972")))->destroy_data = true;
    docs[4] = doc_new();
    doc_add_field(docs[4], df_add_data(df_new(author), 
			estrdup("J. G. Farrell")))->destroy_data = true;
    doc_add_field(docs[4], df_add_data(df_new(title), 
			estrdup("The Siege of Krishnapur")))->destroy_data = true;
    doc_add_field(docs[4], df_add_data(df_new(year), 
			estrdup("1973")))->destroy_data = true;
    docs[5] = doc_new();
    doc_add_field(docs[5], df_add_data(df_new(author), 
			estrdup("Stanley Middleton")))->destroy_data = true;
    doc_add_field(docs[5], df_add_data(df_new(title), 
			estrdup("Holiday")))->destroy_data = true;
    doc_add_field(docs[5], df_add_data(df_new(year), 
			estrdup("1974")))->destroy_data = true;
    docs[6] = doc_new();
    doc_add_field(docs[6], df_add_data(df_new(author), 
			estrdup("Nadine Gordimer")))->destroy_data = true;
    doc_add_field(docs[6], df_add_data(df_new(title), 
			estrdup("The Conservationist")))->destroy_data = true;
    doc_add_field(docs[6], df_add_data(df_new(year), 
			estrdup("1974")))->destroy_data = true;
    docs[7] = doc_new();
    doc_add_field(docs[7], df_add_data(df_new(author), 
			estrdup("Ruth Prawer Jhabvala")))->destroy_data = true;
    doc_add_field(docs[7], df_add_data(df_new(title), 
			estrdup("Heat and Dust")))->destroy_data = true;
    doc_add_field(docs[7], df_add_data(df_new(year), 
			estrdup("1975")))->destroy_data = true;
    docs[8] = doc_new();
    doc_add_field(docs[8], df_add_data(df_new(author), 
			estrdup("David Storey")))->destroy_data = true;
    doc_add_field(docs[8], df_add_data(df_new(title), 
			estrdup("Saville")))->destroy_data = true;
    doc_add_field(docs[8], df_add_data(df_new(year), 
			estrdup("1976")))->destroy_data = true;
    docs[9] = doc_new();
    doc_add_field(docs[9], df_add_data(df_new(author), 
			estrdup("Paul Scott")))->destroy_data = true;
    doc_add_field(docs[9], df_add_data(df_new(title), 
			estrdup("Staying On")))->destroy_data = true;
    doc_add_field(docs[9], df_add_data(df_new(year), 
			estrdup("1977")))->destroy_data = true;
    docs[10] = doc_new();
    doc_add_field(docs[10], df_add_data(df_new(author), 
			estrdup("Iris Murdoch")))->destroy_data = true;
    doc_add_field(docs[10], df_add_data(df_new(title), 
			estrdup("The Sea")))->destroy_data = true;
    doc_add_field(docs[10], df_add_data(df_new(year), 
			estrdup("1978")))->destroy_data = true;
    docs[11] = doc_new();
    doc_add_field(docs[11], df_add_data(df_new(author), 
			estrdup("Penelope Fitzgerald")))->destroy_data = true;
    doc_add_field(docs[11], df_add_data(df_new(title), 
			estrdup("Offshore")))->destroy_data = true;
    doc_add_field(docs[11], df_add_data(df_new(year), 
			estrdup("1979")))->destroy_data = true;
    docs[12] = doc_new();
    doc_add_field(docs[12], df_add_data(df_new(author), 
			estrdup("William Golding")))->destroy_data = true;
    doc_add_field(docs[12], df_add_data(df_new(title), 
			estrdup("Rites of Passage")))->destroy_data = true;
    doc_add_field(docs[12], df_add_data(df_new(year), 
			estrdup("1980")))->destroy_data = true;
    docs[13] = doc_new();
    doc_add_field(docs[13], df_add_data(df_new(author), 
			estrdup("Salman Rushdie")))->destroy_data = true;
    doc_add_field(docs[13], df_add_data(df_new(title), 
			estrdup("Midnight's Children")))->destroy_data = true;
    doc_add_field(docs[13], df_add_data(df_new(year), 
			estrdup("1981")))->destroy_data = true;
    docs[14] = doc_new();
    doc_add_field(docs[14], df_add_data(df_new(author), 
			estrdup("Thomas Keneally")))->destroy_data = true;
    doc_add_field(docs[14], df_add_data(df_new(title), 
			estrdup("Schindler's Ark")))->destroy_data = true;
    doc_add_field(docs[14], df_add_data(df_new(year), 
			estrdup("1982")))->destroy_data = true;
    docs[15] = doc_new();
    doc_add_field(docs[15], df_add_data(df_new(author), 
			estrdup("J. M. Coetzee")))->destroy_data = true;
    doc_add_field(docs[15], df_add_data(df_new(title), 
			estrdup("Life and Times of Michael K")))->destroy_data = true;
    doc_add_field(docs[15], df_add_data(df_new(year), 
			estrdup("1983")))->destroy_data = true;
    docs[16] = doc_new();
    doc_add_field(docs[16], df_add_data(df_new(author), 
			estrdup("Anita Brookner")))->destroy_data = true;
    doc_add_field(docs[16], df_add_data(df_new(title), 
			estrdup("Hotel du Lac")))->destroy_data = true;
    doc_add_field(docs[16], df_add_data(df_new(year), 
			estrdup("1984")))->destroy_data = true;
    docs[17] = doc_new();
    doc_add_field(docs[17], df_add_data(df_new(author), 
			estrdup("Keri Hulme")))->destroy_data = true;
    doc_add_field(docs[17], df_add_data(df_new(title), 
			estrdup("The Bone People")))->destroy_data = true;
    doc_add_field(docs[17], df_add_data(df_new(year), 
			estrdup("1985")))->destroy_data = true;
    docs[18] = doc_new();
    doc_add_field(docs[18], df_add_data(df_new(author), 
			estrdup("Kingsley Amis")))->destroy_data = true;
    doc_add_field(docs[18], df_add_data(df_new(title), 
			estrdup("The Old Devils")))->destroy_data = true;
    doc_add_field(docs[18], df_add_data(df_new(year), 
			estrdup("1986")))->destroy_data = true;
    docs[19] = doc_new();
    doc_add_field(docs[19], df_add_data(df_new(author), 
			estrdup("Penelope Lively")))->destroy_data = true;
    doc_add_field(docs[19], df_add_data(df_new(title), 
			estrdup("Moon Tiger")))->destroy_data = true;
    doc_add_field(docs[19], df_add_data(df_new(year), 
			estrdup("1987")))->destroy_data = true;
    docs[20] = doc_new();
    doc_add_field(docs[20], df_add_data(df_new(author), 
			estrdup("Peter Carey")))->destroy_data = true;
    doc_add_field(docs[20], df_add_data(df_new(title), 
			estrdup("Oscar and Lucinda")))->destroy_data = true;
    doc_add_field(docs[20], df_add_data(df_new(year), 
			estrdup("1988")))->destroy_data = true;
    docs[21] = doc_new();
    doc_add_field(docs[21], df_add_data(df_new(author), 
			estrdup("Kazuo Ishiguro")))->destroy_data = true;
    doc_add_field(docs[21], df_add_data(df_new(title), 
			estrdup("The Remains of the Day")))->destroy_data = true;
    doc_add_field(docs[21], df_add_data(df_new(year), 
			estrdup("1989")))->destroy_data = true;
    docs[22] = doc_new();
    doc_add_field(docs[22], df_add_data(df_new(author), 
			estrdup("A. S. Byatt")))->destroy_data = true;
    doc_add_field(docs[22], df_add_data(df_new(title), 
			estrdup("Possession")))->destroy_data = true;
    doc_add_field(docs[22], df_add_data(df_new(year), 
			estrdup("1990")))->destroy_data = true;
    docs[23] = doc_new();
    doc_add_field(docs[23], df_add_data(df_new(author), 
			estrdup("Ben Okri")))->destroy_data = true;
    doc_add_field(docs[23], df_add_data(df_new(title), 
			estrdup("The Famished Road")))->destroy_data = true;
    doc_add_field(docs[23], df_add_data(df_new(year), 
			estrdup("1991")))->destroy_data = true;
    docs[24] = doc_new();
    doc_add_field(docs[24], df_add_data(df_new(author), 
			estrdup("Michael Ondaatje")))->destroy_data = true;
    doc_add_field(docs[24], df_add_data(df_new(title), 
			estrdup("The English Patient")))->destroy_data = true;
    doc_add_field(docs[24], df_add_data(df_new(year), 
			estrdup("1992")))->destroy_data = true;
    docs[25] = doc_new();
    doc_add_field(docs[25], df_add_data(df_new(author), 
			estrdup("Barry Unsworth")))->destroy_data = true;
    doc_add_field(docs[25], df_add_data(df_new(title), 
			estrdup("Sacred Hunger")))->destroy_data = true;
    doc_add_field(docs[25], df_add_data(df_new(year), 
			estrdup("1992")))->destroy_data = true;
    docs[26] = doc_new();
    doc_add_field(docs[26], df_add_data(df_new(author), 
			estrdup("Roddy Doyle")))->destroy_data = true;
    doc_add_field(docs[26], df_add_data(df_new(title), 
			estrdup("Paddy Clarke Ha Ha Ha")))->destroy_data = true;
    doc_add_field(docs[26], df_add_data(df_new(year), 
			estrdup("1993")))->destroy_data = true;
    docs[27] = doc_new();
    doc_add_field(docs[27], df_add_data(df_new(author), 
			estrdup("James Kelman")))->destroy_data = true;
    doc_add_field(docs[27], df_add_data(df_new(title), 
			estrdup("How Late It Was, How Late")))->destroy_data = true;
    doc_add_field(docs[27], df_add_data(df_new(year), 
			estrdup("1994")))->destroy_data = true;
    docs[28] = doc_new();
    doc_add_field(docs[28], df_add_data(df_new(author), 
			estrdup("Pat Barker")))->destroy_data = true;
    doc_add_field(docs[28], df_add_data(df_new(title), 
			estrdup("The Ghost Road")))->destroy_data = true;
    doc_add_field(docs[28], df_add_data(df_new(year), 
			estrdup("1995")))->destroy_data = true;
    docs[29] = doc_new();
    doc_add_field(docs[29], df_add_data(df_new(author), 
			estrdup("Graham Swift")))->destroy_data = true;
    doc_add_field(docs[29], df_add_data(df_new(title), 
			estrdup("Last Orders")))->destroy_data = true;
    doc_add_field(docs[29], df_add_data(df_new(year), 
			estrdup("1996")))->destroy_data = true;
    docs[30] = doc_new();
    doc_add_field(docs[30], df_add_data(df_new(author), 
			estrdup("Arundati Roy")))->destroy_data = true;
    doc_add_field(docs[30], df_add_data(df_new(title), 
			estrdup("The God of Small Things")))->destroy_data = true;
    doc_add_field(docs[30], df_add_data(df_new(year), 
			estrdup("1997")))->destroy_data = true;
    docs[31] = doc_new();
    doc_add_field(docs[31], df_add_data(df_new(author), 
			estrdup("Ian McEwan")))->destroy_data = true;
    doc_add_field(docs[31], df_add_data(df_new(title), 
			estrdup("Amsterdam")))->destroy_data = true;
    doc_add_field(docs[31], df_add_data(df_new(year), 
			estrdup("1998")))->destroy_data = true;
    docs[32] = doc_new();
    doc_add_field(docs[32], df_add_data(df_new(author), 
			estrdup("J. M. Coetzee")))->destroy_data = true;
    doc_add_field(docs[32], df_add_data(df_new(title), 
			estrdup("Disgrace")))->destroy_data = true;
    doc_add_field(docs[32], df_add_data(df_new(year), 
			estrdup("1999")))->destroy_data = true;
    docs[33] = doc_new();
    doc_add_field(docs[33], df_add_data(df_new(author), 
			estrdup("Margaret Atwood")))->destroy_data = true;
    doc_add_field(docs[33], df_add_data(df_new(title), 
			estrdup("The Blind Assassin")))->destroy_data = true;
    doc_add_field(docs[33], df_add_data(df_new(year), 
			estrdup("2000")))->destroy_data = true;
    docs[34] = doc_new();
    doc_add_field(docs[34], df_add_data(df_new(author), 
			estrdup("Peter Carey")))->destroy_data = true;
    doc_add_field(docs[34], df_add_data(df_new(title), 
			estrdup("True History of the Kelly Gang")))->destroy_data = true;
    doc_add_field(docs[34], df_add_data(df_new(year), 
			estrdup("2001")))->destroy_data = true;
    docs[35] = doc_new();
    doc_add_field(docs[35], df_add_data(df_new(author), 
			estrdup("Yann Martel")))->destroy_data = true;
    doc_add_field(docs[35], df_add_data(df_new(title), 
			estrdup("The Life of Pi")))->destroy_data = true;
    doc_add_field(docs[35], df_add_data(df_new(year), 
			estrdup("2002")))->destroy_data = true;
    docs[36] = doc_new();
    doc_add_field(docs[36], df_add_data(df_new(author), 
			estrdup("DBC Pierre")))->destroy_data = true;
    doc_add_field(docs[36], df_add_data(df_new(title), 
			estrdup("Vernon God Little")))->destroy_data = true;
    doc_add_field(docs[36], df_add_data(df_new(year), 
			estrdup("2003")))->destroy_data = true;

    return docs;
}
*/

#define IR_TEST_DOC_CNT 256

Document **prep_ir_test_docs()
{
    int i;
    char buf[2000] = "";
    Document **docs = ALLOC_N(Document *, IR_TEST_DOC_CNT);
    docs[0] = doc_new();
    doc_add_field(docs[0], df_add_data(df_new(body), 
			estrdup("Where is Wally")))->destroy_data = true;
    doc_add_field(docs[0], df_add_data(df_new(changing_field), 
			estrdup("word3 word4 word1 word2 word1 word3 word4 word1 "
                    "word3 word3")))->destroy_data = true;
    docs[1] = doc_new();
    doc_add_field(docs[1], df_add_data(df_new(body), 
			estrdup("Some Random Sentence read")))->destroy_data = true;
    doc_add_field(docs[1], df_add_data(df_new(tag), 
			estrdup("id_test")))->destroy_data = true;
    docs[2] = doc_new();
    doc_add_field(docs[2], df_add_data(df_new(body), 
			estrdup("Some read Random Sentence read")))->destroy_data = true;
    docs[3] = doc_new();
    doc_add_field(docs[3], df_add_data(df_new(title), 
			estrdup("War And Peace")))->destroy_data = true;
    doc_add_field(docs[3], df_add_data(df_new(body), 
			estrdup("word3 word4 word1 word2 word1 "
                    "word3 word4 word1 word3 word3")))->destroy_data = true;
    doc_add_field(docs[3], df_add_data(df_new(author), 
			estrdup("Leo Tolstoy")))->destroy_data = true;
    doc_add_field(docs[3], df_add_data(df_new(year), 
			estrdup("1865")))->destroy_data = true;
    doc_add_field(docs[3], df_add_data(df_new(text), 
			estrdup("more text which is not stored")))->destroy_data = true;
    docs[4] = doc_new();
    doc_add_field(docs[4], df_add_data(df_new(body), 
			estrdup("Some Random Sentence")))->destroy_data = true;
    docs[5] = doc_new();
    doc_add_field(docs[5], df_add_data(df_new(body), 
			estrdup("Here's Wally")))->destroy_data = true;
    docs[6] = doc_new();
    doc_add_field(docs[6], df_add_data(df_new(body), 
			estrdup("Some Random Sentence read read read read"
                    )))->destroy_data = true;
    docs[7] = doc_new();
    doc_add_field(docs[7], df_add_data(df_new(body), 
			estrdup("Some Random Sentence")))->destroy_data = true;
    docs[8] = doc_new();
    doc_add_field(docs[8], df_add_data(df_new(body), 
			estrdup("Some Random Sentence")))->destroy_data = true;
    docs[9] = doc_new();
    doc_add_field(docs[9], df_add_data(df_new(body), 
			estrdup("read Some Random Sentence read this will be used after "
                    "unfinished next position read")))->destroy_data = true;
    docs[10] = doc_new();
    doc_add_field(docs[10], df_add_data(df_new(body), 
			estrdup("Some read Random Sentence")))->destroy_data = true;
    doc_add_field(docs[10], df_add_data(df_new(changing_field), 
			estrdup("word3 word4 word1 word2 word1 word3 word4 word1 word3 "
                    "word3")))->destroy_data = true;
    docs[11] = doc_new();
    doc_add_field(docs[11], df_add_data(df_new(body), 
			estrdup("And here too. Well, maybe Not")))->destroy_data = true;
    docs[12] = doc_new();
    doc_add_field(docs[12], df_add_data(df_new(body), 
			estrdup("Some Random Sentence")))->destroy_data = true;
    docs[13] = doc_new();
    doc_add_field(docs[13], df_add_data(df_new(body), 
			estrdup("Some Random Sentence")))->destroy_data = true;
    docs[14] = doc_new();
    doc_add_field(docs[14], df_add_data(df_new(body), 
			estrdup("Some Random Sentence")))->destroy_data = true;
    docs[15] = doc_new();
    doc_add_field(docs[15], df_add_data(df_new(body), 
			estrdup("Some read Random Sentence")))->destroy_data = true;
    docs[16] = doc_new();
    doc_add_field(docs[16], df_add_data(df_new(body), 
			estrdup("Some Random read read Sentence")))->destroy_data = true;
    docs[17] = doc_new();
    doc_add_field(docs[17], df_add_data(df_new(body), 
			estrdup("Some Random read Sentence")))->destroy_data = true;
    doc_add_field(docs[17], df_add_data(df_new(changing_field), 
			estrdup("word3 word4 word1 word2 word1 word3 word4 word1 word3 "
                    "word3")))->destroy_data = true;
    docs[18] = doc_new();
    doc_add_field(docs[18], df_add_data(df_new(body), 
			estrdup("Wally Wally Wally")))->destroy_data = true;
    docs[19] = doc_new();
    doc_add_field(docs[19], df_add_data(df_new(body), 
			estrdup("Some Random Sentence")))->destroy_data = true;
    doc_add_field(docs[19], df_add_data(df_new(changing_field), 
			estrdup("word3 word4 word1 word2 word1 word3 word4 word1 word3 "
                    "word3")))->destroy_data = true;
    docs[20] = doc_new();
    doc_add_field(docs[20], df_add_data(df_new(body), 
			estrdup("Wally is where Wally usually likes to go. Wally Mart! "
                    "Wally likes shopping there for Where's Wally books. "
                    "Wally likes to read")))->destroy_data = true;
    doc_add_field(docs[20], df_add_data(df_new(changing_field), 
			estrdup("word3 word4 word1 word2 word1 word3 word4 word1 word3 "
                    "word3")))->destroy_data = true;
    docs[21] = doc_new();
    doc_add_field(docs[21], df_add_data(df_new(body), 
			estrdup("Some Random Sentence read read read and more read read "
                    "read")))->destroy_data = true;
    doc_add_field(docs[21], df_add_data(df_new(changing_field), 
			estrdup("word3 word4 word1 word2 word1 word3 word4 word1 word3 "
                    "word3")))->destroy_data = true;

    for (i = 1; i < 22; i++) {
        strcat(buf, "skip ");
    }
    for (i = 22; i < IR_TEST_DOC_CNT; i++) {
        strcat(buf, "skip ");
        docs[i] = doc_new();
        doc_add_field(docs[i], df_add_data(df_new(text), 
			estrdup(buf)))->destroy_data = true;
    }
    return docs;
}


static void test_fld_inverter(tst_case *tc, void *data)
{
    Store *store = (Store *)data;
    FieldInfos *fis = prep_all_fis();
    HashTable *plists;
    HashTable *curr_plists;
    Posting *p;
    PostingList *pl;
    DocWriter *dw;
    IndexWriter *iw;

    index_create(store, fis);
    iw = iw_open(store, whitespace_analyzer_new(true), NULL);
    dw = dw_open(iw, "_0");

    DocField *df = df_new("no tv");
    df_add_data(df, "one two three four five two three four five three "
                "four five four five");
    df_add_data(df, "ichi ni san yon go ni san yon go san yon go yon go go");
    df_add_data(df, "The quick brown fox jumped over five lazy dogs");

    curr_plists = dw_invert_field(dw, dw_get_fld_inv(dw, df->name), df);

    Aiequal(18, curr_plists->size);
    
    plists = ((FieldInverter *)h_get(dw->fields, df->name))->plists;


    pl = h_get(curr_plists, "one");
    if (Apnotnull(pl)) {
        Asequal("one", pl->term);
        Aiequal(3, pl->term_len);

        p = pl->last;
        Aiequal(1, p->freq);
        Apequal(p->first_occ, pl->last_occ);
        Apnull(p->first_occ->next);
        Aiequal(0, p->first_occ->pos);
        Apequal(pl, ((PostingList *)h_get(plists, "one")));
    }

    pl = h_get(curr_plists, "five");
    if (Apnotnull(pl)) {
        Asequal("five", pl->term);
        Aiequal(4, pl->term_len);
        Apnull(pl->last_occ->next);
        p = pl->last;
        Aiequal(5, p->freq);
        Aiequal(4, p->first_occ->pos);
        Aiequal(8, p->first_occ->next->pos);
        Aiequal(11, p->first_occ->next->next->pos);
        Aiequal(13, p->first_occ->next->next->next->pos);
        Aiequal(35, p->first_occ->next->next->next->next->pos);
        Apequal(pl, ((PostingList *)h_get(plists, "five")));
    }

    df_destroy(df);

    df = df_new("no tv");
    df_add_data(df, "seven new words and six old ones");
    df_add_data(df, "ichi ni one two quick dogs");

    dw->doc_num++;
    dw_reset_postings(dw->curr_plists);

    curr_plists = dw_invert_field(dw, dw_get_fld_inv(dw, df->name), df);

    Aiequal(13, curr_plists->size);

    pl = h_get(curr_plists, "one");
    if (Apnotnull(pl)) {
        Asequal("one", pl->term);
        Aiequal(3, pl->term_len);

        p = pl->first;
        Aiequal(1, p->freq);
        Apnull(p->first_occ->next);
        Aiequal(0, p->first_occ->pos);

        p = pl->last;
        Aiequal(1, p->freq);
        Apequal(p->first_occ, pl->last_occ);
        Apnull(p->first_occ->next);
        Aiequal(9, p->first_occ->pos);
        Apequal(pl, ((PostingList *)h_get(plists, "one")));
    }

    df_destroy(df);

    dw_close(dw);
    iw_close(iw);
    fis_destroy(fis);
}

extern int pl_cmp(Posting *p1, Posting *p2);

#define NUM_POSTINGS TEST_WORD_LIST_SIZE
static void test_postings_sorter(tst_case *tc, void *data)
{
    int i;
    PostingList plists[NUM_POSTINGS], *p_ptr[NUM_POSTINGS];
    (void)data;
    for (i = 0; i < NUM_POSTINGS; i++) {
        plists[i].term = (char *)test_word_list[i];
        p_ptr[i] = &plists[i];
    }

    qsort(p_ptr, NUM_POSTINGS, sizeof(PostingList *),
          (int (*)(const void *, const void *))&pl_cmp);

    for (i = 1; i < NUM_POSTINGS; i++) {
        Assert(strcmp(p_ptr[i - 1]->term, p_ptr[i]->term) <= 0,
               "\"%s\" > \"%s\"", p_ptr[i - 1]->term, p_ptr[i]->term);
    }
}

#define NUM_STDE_TEST_DOCS 50
#define MAX_TEST_WORDS 1000

static void prep_stde_test_docs(Document **docs, int doc_cnt, int num_words,
                            FieldInfos *fis)
{
    int i, j;
    char buf[num_words * (TEST_WORD_LIST_MAX_LEN + 1)];
    for (i = 0; i < doc_cnt; i++) {
        docs[i] = doc_new();
        for (j = 0; j < fis->size; j++) {
            if ((rand() % 2) == 0) {
                DocField *df = df_new(fis->fields[j]->name);
                df_add_data(df, estrdup(make_random_string(buf, num_words)));
                df->destroy_data = true;
                doc_add_field(docs[i], df);
            }
        }
    }
}

static void prep_test_1seg_index(Store *store, Document **docs,
                                 int doc_cnt, FieldInfos *fis)
{
    int i;
    DocWriter *dw;
    IndexWriter *iw;

    index_create(store, fis);
    iw = iw_open(store, whitespace_analyzer_new(false), NULL);

    dw = dw_open(iw, "_0");

    for (i = 0; i < doc_cnt; i++) {
        dw_add_doc(dw, docs[i]);
    }

    dw_close(dw);
    iw_close(iw);
}

static void test_segment_term_doc_enum(tst_case *tc, void *data)
{
    int i, j;
    Store *store = (Store *)data;
    FieldInfos *fis = prep_all_fis();
    FieldInfo *fi;
    SegmentFieldIndex *sfi;
    TermInfosReader *tir;
    InStream *frq_in, *prx_in;
    BitVector *bv = NULL;
    TermDocEnum *tde, *tde_reader, *tde_skip_to;
    char buf[TEST_WORD_LIST_MAX_LEN + 1];
    DocField *df;
    Document *docs[NUM_STDE_TEST_DOCS], *doc;

    prep_stde_test_docs(docs, NUM_STDE_TEST_DOCS, MAX_TEST_WORDS, fis);
    prep_test_1seg_index(store, docs, NUM_STDE_TEST_DOCS, fis);

    sfi = sfi_open(store, "_0");
    tir = tir_open(store, sfi, "_0");
    frq_in = store->open_input(store, "_0.frq");
    prx_in = store->open_input(store, "_0.prx");
    tde = stde_new(tir, frq_in, bv);
    tde_reader = stde_new(tir, frq_in, bv);
    tde_skip_to = stde_new(tir, frq_in, bv);

    fi = fis_get_field(fis, "tv");
    for (i = 0; i < 300; i++) {
        int cnt = 0, ind = 0, doc_nums[3], freqs[3];
        const char *word = test_word_list[rand()%TEST_WORD_LIST_SIZE];
        tde->seek(tde, fi->number, word);
        tde_reader->seek(tde_reader, fi->number, word);
        while (tde->next(tde)) {
            if (cnt == ind) {
                cnt = tde_reader->read(tde_reader, doc_nums, freqs, 3);
                ind = 0;
            }
            Aiequal(doc_nums[ind], tde->doc_num(tde));
            Aiequal(freqs[ind], tde->freq(tde));
            ind++;

            doc = docs[tde->doc_num(tde)];
            df = doc_get_field(doc, fi->name);
            if (Apnotnull(df)) {
                Assert(strstr((char *)df->data[0], word) != NULL,
                       "%s not found in doc[%d]\n\"\"\"\n%s\n\"\"\"\n",
                       word, tde->doc_num(tde), df->data[0]);
            }
            tde_skip_to->seek(tde_skip_to, fi->number, word);
            Atrue(tde_skip_to->skip_to(tde_skip_to, tde->doc_num(tde)));
            Aiequal(tde->doc_num(tde), tde_skip_to->doc_num(tde_skip_to));
            Aiequal(tde->freq(tde), tde_skip_to->freq(tde_skip_to));
        }
        Aiequal(ind, cnt);
    }
    tde->close(tde);
    tde_reader->close(tde_reader);
    tde_skip_to->close(tde_skip_to);


    tde = stpe_new(tir, frq_in, prx_in, bv);
    tde_skip_to = stpe_new(tir, frq_in, prx_in, bv);

    fi = fis_get_field(fis, "tv+offsets");
    for (i = 0; i < 200; i++) {
        const char *word = test_word_list[rand()%TEST_WORD_LIST_SIZE];
        tde->seek(tde, fi->number, word);
        while (tde->next(tde)) {
            tde_skip_to->seek(tde_skip_to, fi->number, word);
            Atrue(tde_skip_to->skip_to(tde_skip_to, tde->doc_num(tde)));
            Aiequal(tde->doc_num(tde), tde_skip_to->doc_num(tde_skip_to));
            Aiequal(tde->freq(tde), tde_skip_to->freq(tde_skip_to));

            doc = docs[tde->doc_num(tde)];
            df = doc_get_field(doc, fi->name);
            if (Apnotnull(df)) {
                Assert(strstr((char *)df->data[0], word) != NULL,
                       "%s not found in doc[%d]\n\"\"\"\n%s\n\"\"\"\n",
                       word, tde->doc_num(tde), df->data[0]);
                for (j = tde->freq(tde); j > 0; j--) {
                    int pos = tde->next_position(tde), t;
                    Aiequal(pos, tde_skip_to->next_position(tde_skip_to));
                    Asequal(word, get_nth_word(df->data[0], buf, pos, &t, &t));
                }
            }
        }

    }
    tde->close(tde);
    tde_skip_to->close(tde_skip_to);

    for (i = 0; i < NUM_STDE_TEST_DOCS; i++) {
        doc_destroy(docs[i]);
    }
    fis_destroy(fis);
    is_close(frq_in);
    is_close(prx_in);
    tir_close(tir);
    sfi_close(sfi);
}

static void test_segment_tde_deleted_docs(tst_case *tc, void *data)
{
    int i, doc_num_expected;
    Store *store = (Store *)data;
    FieldInfos *fis = fis_new(STORE_NO, INDEX_YES, TERM_VECTOR_NO);
    DocWriter *dw;
    Document *doc;
    IndexWriter *iw;
    SegmentFieldIndex *sfi;
    TermInfosReader *tir;
    InStream *frq_in, *prx_in;
    BitVector *bv = bv_new();
    TermDocEnum *tde;

    index_create(store, fis);
    iw = iw_open(store, whitespace_analyzer_new(false), NULL);
    dw = dw_open(iw, "_0");

    for (i = 0; i < NUM_STDE_TEST_DOCS; i++) {
        doc = doc_new();
        if ((rand() % 2) == 0) {
            bv_set(bv, i);
            Aiequal(1, bv_get(bv, i));
            doc_add_field(doc, df_add_data(df_new("f"), "word word"));
        }
        else {
            doc_add_field(doc, df_add_data(df_new("f"), "word word word"));
        }
        dw_add_doc(dw, doc);
        doc_destroy(doc);
    }
    Aiequal(NUM_STDE_TEST_DOCS, dw->doc_num);
    dw_close(dw);
    iw_close(iw);

    sfi = sfi_open(store, "_0");
    tir = tir_open(store, sfi, "_0");
    frq_in = store->open_input(store, "_0.frq");
    prx_in = store->open_input(store, "_0.prx");
    tde = stpe_new(tir, frq_in, prx_in, bv);

    tde->seek(tde, 0, "word");
    doc_num_expected = 0;
    while (tde->next(tde)) {
        while (bv_get(bv, doc_num_expected)) {
            doc_num_expected++;
        }
        Aiequal(doc_num_expected, tde->doc_num(tde));
        if (Aiequal(3, tde->freq(tde))) {
            for (i = 0; i < 3; i++) {
                Aiequal(i, tde->next_position(tde));
            }
        }
        doc_num_expected++;
    }
    tde->close(tde);

    bv_destroy(bv);
    fis_destroy(fis);
    is_close(frq_in);
    is_close(prx_in);
    tir_close(tir);
    sfi_close(sfi);
}

void write_ir_test_docs(Store *store, int is_for_segment_reader)
{
    int i;
    IndexWriter *iw;
    Document **docs;
    FieldInfos *fis = fis_new(STORE_YES, INDEX_YES,
                              TERM_VECTOR_WITH_POSITIONS_OFFSETS);
    fis_add_field(fis, fi_new("author", STORE_YES, INDEX_YES,
                              TERM_VECTOR_WITH_POSITIONS));
    fis_add_field(fis, fi_new("title", STORE_YES, INDEX_UNTOKENIZED,
                              TERM_VECTOR_WITH_OFFSETS));
    fis_add_field(fis, fi_new("year", STORE_YES, INDEX_UNTOKENIZED,
                              TERM_VECTOR_NO));
    fis_add_field(fis, fi_new("text", STORE_NO, INDEX_YES,
                              TERM_VECTOR_NO));
    index_create(store, fis);
    fis_destroy(fis);

    iw = iw_open(store, whitespace_analyzer_new(false), &default_config);
    docs = prep_ir_test_docs();

    for (i = 0; i < IR_TEST_DOC_CNT; i++) {
        iw_add_doc(iw, docs[i]);
    }

    if (is_for_segment_reader) {
        //iw_optimize(iw);
    }
    iw_close(iw);

    destroy_docs(docs, IR_TEST_DOC_CNT);
}
    
static void test_ir_basic_ops(tst_case *tc, void *data)
{
    IndexReader *ir = (IndexReader *)data;

    Aiequal(IR_TEST_DOC_CNT, ir->num_docs(ir));
    Aiequal(IR_TEST_DOC_CNT, ir->max_doc(ir));

    Aiequal(4, ir->doc_freq(ir, fis_get_field(ir->fis, body)->number, "Wally"));
}

void test_ir_term_docpos_enum_skip_to(tst_case *tc, TermDocEnum *tde, int field_num)
{
    /* test skip_to working skip interval */
    tde->seek(tde, field_num, "skip");

    Atrue(tde->skip_to(tde, 10));
    Aiequal(22, tde->doc_num(tde));
    Aiequal(22, tde->freq(tde));

    Atrue(tde->skip_to(tde, 100));
    Aiequal(100, tde->doc_num(tde));
    Aiequal(100, tde->freq(tde));

    tde->seek(tde, field_num, "skip");
    Atrue(tde->skip_to(tde, 85));
    Aiequal(85, tde->doc_num(tde));
    Aiequal(85, tde->freq(tde));

    Atrue(tde->skip_to(tde, 200));
    Aiequal(200, tde->doc_num(tde));
    Aiequal(200, tde->freq(tde));

    Atrue(tde->skip_to(tde, 255));
    Aiequal(255, tde->doc_num(tde));
    Aiequal(255, tde->freq(tde));

    Atrue(!tde->skip_to(tde, 256));

    tde->seek(tde, field_num, "skip");

    Atrue(!tde->skip_to(tde, 256));
}

#define AA3(x, a, b, c) x[0] = a; x[1] = b; x[2] = c;

void test_ir_term_doc_enum(tst_case *tc, void *data)
{
    IndexReader *ir = (IndexReader *)data;

    TermDocEnum *tde;
    Document *doc =
        ir_get_doc_with_term(ir, tag, "id_test");
    int docs[3], expected_docs[3];
    int freqs[3], expected_freqs[3];

    Apnotnull(doc);
    Asequal("id_test", doc_get_field(doc, tag)->data[0]); 
    Asequal("Some Random Sentence read", doc_get_field(doc, body)->data[0]); 
    doc_destroy(doc);

    /* test scanning */
    tde = ir_term_docs_for(ir, body, "Wally");

    Atrue(tde->next(tde));
    Aiequal(0, tde->doc_num(tde));
    Aiequal(1, tde->freq(tde));

    Atrue(tde->next(tde));
    Aiequal(5, tde->doc_num(tde));
    Aiequal(1, tde->freq(tde));

    Atrue(tde->next(tde));
    Aiequal(18, tde->doc_num(tde));
    Aiequal(3, tde->freq(tde));

    Atrue(tde->next(tde));
    Aiequal(20, tde->doc_num(tde));
    Aiequal(6, tde->freq(tde));
    Atrue(! tde->next(tde));

    /* test fast read. Use a small array to exercise repeat read */
    tde->seek(tde, fis_get_field(ir->fis, body)->number, "read");
    Aiequal(3, tde->read(tde, docs, freqs, 3));
    AA3(expected_freqs, 1, 2, 4);
    AA3(expected_docs, 1, 2, 6);
    Aaiequal(expected_docs, docs, 3);
    Aaiequal(expected_freqs, freqs, 3);

    Aiequal(3, tde->read(tde, docs, freqs, 3));
    AA3(expected_docs, 9, 10, 15);
    AA3(expected_freqs, 3, 1, 1);
    Aaiequal(expected_docs, docs, 3);
    Aaiequal(expected_freqs, freqs, 3);

    Aiequal(3, tde->read(tde, docs, freqs, 3));
    AA3(expected_docs, 16, 17, 20);
    AA3(expected_freqs, 2, 1, 1);
    Aaiequal(expected_docs, docs, 3);
    Aaiequal(expected_freqs, freqs, 3);

    Aiequal(1, tde->read(tde, docs, freqs, 3));
    expected_docs[0] = 21;
    expected_freqs[0] = 6;
    Aaiequal(expected_docs, docs, 1);
    Aaiequal(expected_freqs, freqs, 1);

    Aiequal(0, tde->read(tde, docs, freqs, 3));

    test_ir_term_docpos_enum_skip_to(tc, tde,
                                     fis_get_field(ir->fis, text)->number);
    tde->close(tde);

    /* test term positions */
    tde = ir_term_positions_for(ir, body, "read");
    Atrue(tde->next(tde));
    Aiequal(1, tde->doc_num(tde));
    Aiequal(1, tde->freq(tde));
    Aiequal(3, tde->next_position(tde));

    Atrue(tde->next(tde));
    Aiequal(2, tde->doc_num(tde));
    Aiequal(2, tde->freq(tde));
    Aiequal(1, tde->next_position(tde));
    Aiequal(4, tde->next_position(tde));

    Atrue(tde->next(tde));
    Aiequal(6, tde->doc_num(tde));
    Aiequal(4, tde->freq(tde));
    Aiequal(3, tde->next_position(tde));
    Aiequal(4, tde->next_position(tde));

    Atrue(tde->next(tde));
    Aiequal(9, tde->doc_num(tde));
    Aiequal(3, tde->freq(tde));
    Aiequal(0, tde->next_position(tde));
    Aiequal(4, tde->next_position(tde));

    Atrue(tde->skip_to(tde, 16));
    Aiequal(16, tde->doc_num(tde));
    Aiequal(2, tde->freq(tde));
    Aiequal(2, tde->next_position(tde));

    Atrue(tde->skip_to(tde, 21));
    Aiequal(21, tde->doc_num(tde));
    Aiequal(6, tde->freq(tde));
    Aiequal(3, tde->next_position(tde));
    Aiequal(4, tde->next_position(tde));
    Aiequal(5, tde->next_position(tde));
    Aiequal(8, tde->next_position(tde));
    Aiequal(9, tde->next_position(tde));
    Aiequal(10, tde->next_position(tde));

    Atrue(! tde->next(tde));

    test_ir_term_docpos_enum_skip_to(tc, tde,
                                     fis_get_field(ir->fis, text)->number);
    tde->close(tde);
}

void test_ir_term_vectors(tst_case *tc, void *data)
{ 
    IndexReader *ir = (IndexReader *)data;

    TermVector *tv = ir->term_vector(ir, 3, "body");
    HashTable *tvs;

    Asequal("body", tv->field);
    Aiequal(4, tv->size);
    Asequal("word1", tv->terms[0].text);
    Asequal("word2", tv->terms[1].text);
    Asequal("word3", tv->terms[2].text);
    Asequal("word4", tv->terms[3].text);
    Aiequal(3, tv->terms[0].freq);
    Aiequal(2, tv->terms[0].positions[0]);
    Aiequal(4, tv->terms[0].positions[1]);
    Aiequal(7, tv->terms[0].positions[2]);
    Aiequal(12, tv->terms[0].offsets[0].start);
    Aiequal(17, tv->terms[0].offsets[0].end);
    Aiequal(24, tv->terms[0].offsets[1].start);
    Aiequal(29, tv->terms[0].offsets[1].end);
    Aiequal(42, tv->terms[0].offsets[2].start);
    Aiequal(47, tv->terms[0].offsets[2].end);

    Aiequal(1, tv->terms[1].freq);
    Aiequal(3, tv->terms[1].positions[0]);
    Aiequal(18, tv->terms[1].offsets[0].start);
    Aiequal(23, tv->terms[1].offsets[0].end);

    Aiequal(4, tv->terms[2].freq);
    Aiequal(0, tv->terms[2].positions[0]);
    Aiequal(5, tv->terms[2].positions[1]);
    Aiequal(8, tv->terms[2].positions[2]);
    Aiequal(9, tv->terms[2].positions[3]);
    Aiequal(0, tv->terms[2].offsets[0].start);
    Aiequal(5, tv->terms[2].offsets[0].end);
    Aiequal(30, tv->terms[2].offsets[1].start);
    Aiequal(35, tv->terms[2].offsets[1].end);
    Aiequal(48, tv->terms[2].offsets[2].start);
    Aiequal(53, tv->terms[2].offsets[2].end);
    Aiequal(54, tv->terms[2].offsets[3].start);
    Aiequal(59, tv->terms[2].offsets[3].end);

    Aiequal(2, tv->terms[3].freq);
    Aiequal(1, tv->terms[3].positions[0]);
    Aiequal(6, tv->terms[3].positions[1]);
    Aiequal(6, tv->terms[3].offsets[0].start);
    Aiequal(11, tv->terms[3].offsets[0].end);
    Aiequal(36, tv->terms[3].offsets[1].start);
    Aiequal(41, tv->terms[3].offsets[1].end);

    tv_destroy(tv);

    tvs = ir->term_vectors(ir, 3);
    Aiequal(3, tvs->size);
    tv = h_get(tvs, "author");
    if (Apnotnull(tv)) {
        Asequal("author", tv->field);
        Aiequal(2, tv->size);
        Apnull(tv->terms[0].offsets);
    }
    tv = h_get(tvs, "body");
    if (Apnotnull(tv)) {
        Asequal("body", tv->field);
        Aiequal(4, tv->size);
    }
    tv = h_get(tvs, "title");
    if (Apnotnull(tv)) {
        Asequal("title", tv->field);
        Aiequal(1, tv->size); /* untokenized */
        Asequal("War And Peace", tv->terms[0].text);
        Apnull(tv->terms[0].positions);
        Aiequal(0, tv->terms[0].offsets[0].start);
        Aiequal(13, tv->terms[0].offsets[0].end);
    }
    h_destroy(tvs);
}

void test_ir_get_doc(tst_case *tc, void *data)
{ 
    IndexReader *ir = (IndexReader *)data;
    Document *doc = ir->get_doc(ir, 3);
    DocField *df;
    Aiequal(4, doc->size);

    df = doc_get_field(doc, author);
    Asequal(author, df->name);
    Asequal("Leo Tolstoy", df->data[0]);
    Afequal(df->boost, 1.0);

    df = doc_get_field(doc, body);
    Asequal(body, df->name);
    Asequal("word3 word4 word1 word2 word1 "
            "word3 word4 word1 word3 word3", df->data[0]);
    Afequal(df->boost, 1.0);
    df = doc_get_field(doc, title);
    Asequal(title, df->name);
    Asequal("War And Peace", df->data[0]);
    Afequal(df->boost, 1.0);

    df = doc_get_field(doc, year);
    Asequal(year, df->name);
    Asequal("1865", df->data[0]);
    Afequal(df->boost, 1.0);

    df = doc_get_field(doc, text);
    Apnull(df); /* text is not stored */

    doc_destroy(doc);
}

tst_suite *ts_index(tst_suite * suite)
{
    IndexReader *ir;
    Store *store = open_ram_store();

    srand(5);
    suite = ADD_SUITE(suite);

    tst_run_test(suite, test_fld_inverter, store);
    tst_run_test(suite, test_postings_sorter, NULL);
    tst_run_test(suite, test_segment_term_doc_enum, store);
    tst_run_test(suite, test_segment_tde_deleted_docs, store);

    write_ir_test_docs(store, false);
    ir = ir_open(store);

    tst_run_test(suite, test_ir_basic_ops, ir);
    tst_run_test(suite, test_ir_term_doc_enum, ir);
    tst_run_test(suite, test_ir_term_vectors, ir);
    tst_run_test(suite, test_ir_get_doc, ir);

    ir_close(ir);

    store_deref(store);
    return suite;
}
