#include "index.h"
#include "testhelper.h"
#include "test.h"

static Symbol body, title, text, author, year, changing_field, compressed_field, tag;

static FieldInfos *prep_all_fis()
{
    FieldInfos *fis = fis_new(STORE_NO, INDEX_YES, TERM_VECTOR_NO);
    fis_add_field(fis, fi_new(I("tv"), STORE_NO, INDEX_YES, TERM_VECTOR_YES));
    fis_add_field(fis, fi_new(I("tv un-t"), STORE_NO, INDEX_UNTOKENIZED,
                              TERM_VECTOR_YES));
    fis_add_field(fis, fi_new(I("tv+offsets"), STORE_NO, INDEX_YES,
                              TERM_VECTOR_WITH_OFFSETS));
    fis_add_field(fis, fi_new(I("tv+offsets un-t"), STORE_NO, INDEX_UNTOKENIZED,
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

static FieldInfos *prep_book_fis()
{
    FieldInfos *fis = fis_new(STORE_YES, INDEX_YES,
                              TERM_VECTOR_WITH_POSITIONS_OFFSETS);
    fis_add_field(fis, fi_new(I("year"), STORE_YES, INDEX_NO, TERM_VECTOR_NO));
    return fis;

}

Document *prep_book()
{
    Document *doc = doc_new();
    doc_add_field(doc, df_add_data(df_new(author), 
            estrdup("P.H. Newby")))->destroy_data = true;
    doc_add_field(doc, df_add_data(df_new(title), 
            estrdup("Something To Answer For")))->destroy_data = true;
    doc_add_field(doc, df_add_data(df_new(year), 
            estrdup("1969")))->destroy_data = true;
    return doc;
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

static void add_document_with_fields(IndexWriter *iw, int i)
{
    Document **docs = prep_book_list();
    iw_add_doc(iw, docs[i]);
    destroy_docs(docs, BOOK_LIST_LENGTH);
}

static IndexWriter *create_book_iw_conf(Store *store, const Config *config)
{
    FieldInfos *fis = prep_book_fis();
    index_create(store, fis);
    fis_deref(fis);
    return iw_open(store, whitespace_analyzer_new(false), config);
}

static IndexWriter *create_book_iw(Store *store)
{
    return create_book_iw_conf(store, &default_config);
}

#define IR_TEST_DOC_CNT 256

Document **prep_ir_test_docs()
{
    int i;
    char buf[2000] = "";
    Document **docs = ALLOC_N(Document *, IR_TEST_DOC_CNT);
    DocField *df;

    docs[0] = doc_new();
    doc_add_field(docs[0], df_add_data(df_new(changing_field), 
            estrdup("word3 word4 word1 word2 word1 word3 word4 word1 "
                    "word3 word3")))->destroy_data = true;
    doc_add_field(docs[0], df_add_data(df_new(compressed_field), 
            estrdup("word3 word4 word1 word2 word1 word3 word4 word1 "
                    "word3 word3")))->destroy_data = true;
    doc_add_field(docs[0], df_add_data(df_new(body), 
            estrdup("Where is Wally")))->destroy_data = true;
    docs[1] = doc_new();
    doc_add_field(docs[1], df_add_data(df_new(body), 
            estrdup("Some Random Sentence read")))->destroy_data = true;
    doc_add_field(docs[1], df_add_data(df_new(tag), 
            estrdup("id_test")))->destroy_data = true;
    docs[2] = doc_new();
    doc_add_field(docs[2], df_add_data(df_new(body), 
            estrdup("Some read Random Sentence read")))->destroy_data = true;
    df = df_new(tag);
    df_add_data(df, estrdup("one"));
    df_add_data(df, estrdup("two"));
    df_add_data(df, estrdup("three"));
    df_add_data(df, estrdup("four"));
    doc_add_field(docs[2], df)->destroy_data = true;
    df = df_new(compressed_field);
    df_add_data(df, estrdup("one"));
    df_add_data(df, estrdup("two"));
    df_add_data(df, estrdup("three"));
    df_add_data(df, estrdup("four"));
    doc_add_field(docs[2], df)->destroy_data = true;
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
    doc_add_field(docs[5], df_add_data(df_new(text), 
            estrdup("so_that_norm_can_be_set")))->destroy_data = true;
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
    doc_add_field(docs[12], df_add_data(df_new(title), 
            estrdup("Shawshank Redemption")))->destroy_data = true;
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
    doc_add_field(docs[21], df_add_data(df_new(I("new field")), 
            estrdup("zdata znot zto zbe zfound")))->destroy_data = true;
    doc_add_field(docs[21], df_add_data(df_new(title), 
            estrdup("title_too_long_for_max_word_lengthxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx")))->destroy_data = true;

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

#define NUM_STDE_TEST_DOCS 50
#define MAX_TEST_WORDS 1000

static void prep_stde_test_docs(Document **docs, int doc_cnt, int num_words,
                            FieldInfos *fis)
{
    int i, j;
    char *buf = ALLOC_N(char, num_words * (TEST_WORD_LIST_MAX_LEN + 1));
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
    free(buf);
}

static void prep_test_1seg_index(Store *store, Document **docs,
                                 int doc_cnt, FieldInfos *fis)
{
    int i;
    DocWriter *dw;
    IndexWriter *iw;
    SegmentInfo *si = si_new(estrdup("_0"), doc_cnt, store);

    index_create(store, fis);
    iw = iw_open(store, whitespace_analyzer_new(false), NULL);

    dw = dw_open(iw, si);

    for (i = 0; i < doc_cnt; i++) {
        dw_add_doc(dw, docs[i]);
    }

    dw_close(dw);
    iw_close(iw);
    si_deref(si);
}

/****************************************************************************
 *
 * TermDocEnum
 *
 ****************************************************************************/

static void test_segment_term_doc_enum(TestCase *tc, void *data)
{
    int i, j;
    Store *store = (Store *)data;
    FieldInfos *fis = prep_all_fis();
    FieldInfo *fi;
    SegmentFieldIndex *sfi;
    TermInfosReader *tir;
    int skip_interval;
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
    skip_interval = ((SegmentTermEnum *)tir->orig_te)->skip_interval;
    frq_in = store->open_input(store, "_0.frq");
    prx_in = store->open_input(store, "_0.prx");
    tde = stde_new(tir, frq_in, bv, skip_interval);
    tde_reader = stde_new(tir, frq_in, bv, skip_interval);
    tde_skip_to = stde_new(tir, frq_in, bv, skip_interval);

    fi = fis_get_field(fis, I("tv"));
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

        Atrue(! tde->next(tde));
        Atrue(! tde->next(tde));
        Atrue(! tde->skip_to(tde, 0));
        Atrue(! tde->skip_to(tde, 1000000));
    }
    tde->close(tde);
    tde_reader->close(tde_reader);
    tde_skip_to->close(tde_skip_to);


    tde = stpe_new(tir, frq_in, prx_in, bv, skip_interval);
    tde_skip_to = stpe_new(tir, frq_in, prx_in, bv, skip_interval);

    fi = fis_get_field(fis, I("tv+offsets"));
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
        Atrue(! tde->next(tde));
        Atrue(! tde->next(tde));
        Atrue(! tde->skip_to(tde, 0));
        Atrue(! tde->skip_to(tde, 1000000));

    }
    tde->close(tde);
    tde_skip_to->close(tde_skip_to);

    for (i = 0; i < NUM_STDE_TEST_DOCS; i++) {
        doc_destroy(docs[i]);
    }
    fis_deref(fis);
    is_close(frq_in);
    is_close(prx_in);
    tir_close(tir);
    sfi_close(sfi);
}

const char *double_word = "word word";
const char *triple_word = "word word word";

static void test_segment_tde_deleted_docs(TestCase *tc, void *data)
{
    int i, doc_num_expected, skip_interval;
    Store *store = (Store *)data;
    DocWriter *dw;
    Document *doc;
    IndexWriter *iw = create_book_iw(store);
    SegmentFieldIndex *sfi;
    TermInfosReader *tir;
    InStream *frq_in, *prx_in;
    BitVector *bv = bv_new();
    TermDocEnum *tde;
    SegmentInfo *si = si_new(estrdup("_0"), NUM_STDE_TEST_DOCS, store);

    dw = dw_open(iw, si);

    for (i = 0; i < NUM_STDE_TEST_DOCS; i++) {
        doc = doc_new();
        if ((rand() % 2) == 0) {
            bv_set(bv, i);
            Aiequal(1, bv_get(bv, i));
            doc_add_field(doc, df_add_data(df_new(I("f")), (char *)double_word));
        }
        else {
            doc_add_field(doc, df_add_data(df_new(I("f")), (char *)triple_word));
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
    skip_interval = sfi->skip_interval;
    tde = stpe_new(tir, frq_in, prx_in, bv, skip_interval);

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
    is_close(frq_in);
    is_close(prx_in);
    tir_close(tir);
    sfi_close(sfi);
    si_deref(si);
}

/****************************************************************************
 *
 * Index
 *
 ****************************************************************************/

static void test_index_create(TestCase *tc, void *data)
{
    Store *store = (Store *)data;
    FieldInfos *fis = fis_new(STORE_YES, INDEX_YES, TERM_VECTOR_YES);
    (void)tc;

    store->clear_all(store);
    Assert(!store->exists(store, "segments"),
           "segments shouldn't exist yet");
    index_create(store, fis);
    Assert(store->exists(store, "segments"),
           "segments should now exist");
    fis_deref(fis);
}

static void test_index_version(TestCase *tc, void *data)
{
    u64 version;
    Store *store = (Store *)data;
    IndexWriter *iw = create_book_iw(store);
    IndexReader *ir;

    add_document_with_fields(iw, 0);
    Atrue(index_is_locked(store));  /* writer open, so dir is locked */
    iw_close(iw);
    Atrue(!index_is_locked(store));
    ir = ir_open(store);
    Atrue(!index_is_locked(store)); /* reader only, no lock */
    version = sis_read_current_version(store);
    ir_close(ir);

    /* modify index and check version has been incremented: */
    iw = iw_open(store, whitespace_analyzer_new(false), &default_config);
    add_document_with_fields(iw, 1);
    iw_close(iw);
    ir = ir_open(store);
    Atrue(version < sis_read_current_version(store));
    Atrue(ir_is_latest(ir));
    ir_close(ir);
}

static void test_index_undelete_all_after_close(TestCase *tc, void *data)
{
    Store *store = (Store *)data;
    IndexWriter *iw = create_book_iw(store);
    IndexReader *ir;
    add_document_with_fields(iw, 0);
    add_document_with_fields(iw, 1);
    iw_close(iw);
    ir = ir_open(store);
    ir_delete_doc(ir, 0);
    ir_delete_doc(ir, 1);
    ir_close(ir);
    ir = ir_open(store);
    ir_undelete_all(ir);
    Aiequal(2, ir->num_docs(ir)); /* nothing has really been deleted */
    ir_close(ir);
    ir = ir_open(store);
    Aiequal(2, ir->num_docs(ir)); /* nothing has really been deleted */
    Atrue(ir_is_latest(ir));
    ir_close(ir);
}

/****************************************************************************
 *
 * IndexWriter
 *
 ****************************************************************************/

static void test_fld_inverter(TestCase *tc, void *data)
{
    Store *store = (Store *)data;
    Hash *plists;
    Hash *curr_plists;
    Posting *p;
    PostingList *pl;
    DocWriter *dw;
    IndexWriter *iw = create_book_iw(store);
    DocField *df;

    dw = dw_open(iw, sis_new_segment(iw->sis, 0, iw->store));

    df = df_new(I("no tv"));
    df_add_data(df, "one two three four five two three four five three "
                "four five four five");
    df_add_data(df, "ichi ni san yon go ni san yon go san yon go yon go go");
    df_add_data(df, "The quick brown fox jumped over five lazy dogs");

    curr_plists = dw_invert_field(
        dw,
        dw_get_fld_inv(dw, fis_get_or_add_field(dw->fis, df->name)),
        df);

    Aiequal(18, curr_plists->size);
    
    plists = ((FieldInverter *)h_get_int(
            dw->fields, fis_get_field(dw->fis, df->name)->number))->plists;


    pl = (PostingList *)h_get(curr_plists, "one");
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

    pl = (PostingList *)h_get(curr_plists, "five");
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

    df = df_new(I("no tv"));
    df_add_data(df, "seven new words and six old ones");
    df_add_data(df, "ichi ni one two quick dogs");

    dw->doc_num++;
    dw_reset_postings(dw->curr_plists);

    curr_plists = dw_invert_field(
        dw,
        dw_get_fld_inv(dw, fis_get_or_add_field(dw->fis, df->name)),
        df);

    Aiequal(13, curr_plists->size);

    pl = (PostingList *)h_get(curr_plists, "one");
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
}

#define NUM_POSTINGS TEST_WORD_LIST_SIZE
static void test_postings_sorter(TestCase *tc, void *data)
{
    int i;
    PostingList plists[NUM_POSTINGS], *p_ptr[NUM_POSTINGS];
    (void)data, (void)tc;
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

static void test_iw_add_doc(TestCase *tc, void *data)
{
    Store *store = (Store *)data;
    IndexWriter *iw = create_book_iw(store);
    Document **docs = prep_book_list();

    iw_add_doc(iw, docs[0]);
    Aiequal(1, iw_doc_count(iw));
    Assert(!store->exists(store, "_0.cfs"),
           "data shouldn't have been written yet");
    iw_commit(iw);
    Assert(store->exists(store, "_0.cfs"), "data should now be written");
    iw_close(iw);
    Assert(store->exists(store, "_0.cfs"), "data should still be there");

    iw = iw_open(store, whitespace_analyzer_new(false), &default_config);
    iw_add_doc(iw, docs[1]);
    Aiequal(2, iw_doc_count(iw));
    Assert(!store->exists(store, "_1.cfs"),
           "data shouldn't have been written yet");
    Assert(store->exists(store, "_0.cfs"), "data should still be there");
    iw_commit(iw);
    Assert(store->exists(store, "_1.cfs"), "data should now be written");
    iw_close(iw);
    Assert(store->exists(store, "_1.cfs"), "data should still be there");
    Assert(store->exists(store, "_0.cfs"), "data should still be there");

    destroy_docs(docs, BOOK_LIST_LENGTH);
}

/*
 * Make sure we can open an index for create even when a
 * reader holds it open (this fails pre lock-less
 * commits on windows):
 */
static void test_create_with_reader(TestCase *tc, void *data)
{
    Store *store = open_fs_store(TEST_DIR);
    (void)data;
    IndexWriter *iw;
    IndexReader *ir, *ir2;
    Document *doc = prep_book();
    store->clear_all(store);

    /* add one document & close writer */
    iw = create_book_iw(store);
    iw_add_doc(iw, doc);
    iw_close(iw);

    /* now open reader: */
    ir = ir_open(store);
    Aiequal(1, ir->num_docs(ir));

    /* now open index for create: */
    iw = create_book_iw(store);
    Aiequal(0, iw_doc_count(iw));
    iw_add_doc(iw, doc);
    iw_close(iw);

    Aiequal(1, ir->num_docs(ir));
    ir2 = ir_open(store);
    Aiequal(1, ir2->num_docs(ir));
    ir_close(ir);
    ir_close(ir2);
    store->clear_all(store);
    store_deref(store);
    doc_destroy(doc);
}

/*
 * Simulate a writer that crashed while writing segments
 * file: make sure we can still open the index (ie,
 * gracefully fallback to the previous segments file),
 * and that we can add to the index:
 */
static void test_simulated_crashed_writer(TestCase *tc, void *data)
{
    int i;
    long gen;
    off_t length;
    Store *store = (Store *)data;
    IndexWriter *iw;
    IndexReader *ir;
    char file_name_in[SEGMENT_NAME_MAX_LENGTH];
    char file_name_out[SEGMENT_NAME_MAX_LENGTH];
    InStream *is;
    OutStream *os;
    Document **docs = prep_book_list();
    Config config = default_config;
    config.max_buffered_docs = 3;

    iw = create_book_iw_conf(store, &config);
    for (i = 0; i < BOOK_LIST_LENGTH; i++) {
        iw_add_doc(iw, docs[i]);
    }

    /* close */
    iw_close(iw);

    gen = sis_current_segment_generation(store);
    /* segment generation should be > 1 */
    Atrue(gen > 1);

    /* Make the next segments file, with last byte
     * missing, to simulate a writer that crashed while
     * writing segments file: */
    sis_curr_seg_file_name(file_name_in, store);
    fn_for_generation(file_name_out, SEGMENTS_FILE_NAME, NULL, 1 + gen);
    is = store->open_input(store, file_name_in);
    os = store->new_output(store, file_name_out);
    length = is_length(is);
    for(i = 0; i < length - 1; i++) {
        os_write_byte(os, is_read_byte(is));
    }
    is_close(is);
    os_close(os);

    ir = ir_open(store);
    ir_close(ir);

    iw = iw_open(store, whitespace_analyzer_new(false), &config);

    /* add all books */
    for (i = 0; i < BOOK_LIST_LENGTH; i++) {
        iw_add_doc(iw, docs[i]);
    }

    destroy_docs(docs, BOOK_LIST_LENGTH);
    iw_close(iw);
}

/*
 * Simulate a corrupt index by removing last byte of
 * latest segments file and make sure we get an
 * IOException trying to open the index:
 */
static void test_simulated_corrupt_index1(TestCase *tc, void *data)
{
    int i;
    long gen;
    off_t length;
    Store *store = (Store *)data;
    IndexWriter *iw;
    IndexReader *ir;
    char file_name_in[SEGMENT_NAME_MAX_LENGTH];
    char file_name_out[SEGMENT_NAME_MAX_LENGTH];
    InStream *is;
    OutStream *os;
    Document **docs = prep_book_list();
    Config config = default_config;
    config.max_buffered_docs = 3;

    iw = create_book_iw_conf(store, &config);
    for (i = 0; i < BOOK_LIST_LENGTH; i++) {
        iw_add_doc(iw, docs[i]);
    }

    /* close */
    iw_close(iw);

    gen = sis_current_segment_generation(store);
    /* segment generation should be > 1 */
    Atrue(gen > 1);

    /* Make the next segments file, with last byte
     * missing, to simulate a writer that crashed while
     * writing segments file: */
    sis_curr_seg_file_name(file_name_in, store);
    fn_for_generation(file_name_out, SEGMENTS_FILE_NAME, "", 1 + gen);
    is = store->open_input(store, file_name_in);
    os = store->new_output(store, file_name_out);
    length = is_length(is);
    for(i = 0; i < length - 1; i++) {
        os_write_byte(os, is_read_byte(is));
    }
    is_close(is);
    os_close(os);
    store->remove(store, file_name_in);

    TRY
        ir = ir_open(store);
        ir_close(ir);
        Afail("reader should have failed to open on a crashed index");
        break;
    case IO_ERROR:
        HANDLED();
        break;
    default:
        Afail("reader should have raised an IO_ERROR");
        HANDLED();
    XENDTRY
    destroy_docs(docs, BOOK_LIST_LENGTH);
}

/*
 * Simulate a corrupt index by removing one of the cfs
 * files and make sure we get an IOException trying to
 * open the index:
 */
static void test_simulated_corrupt_index2(TestCase *tc, void *data)
{
    int i;
    long gen;
    Store *store = (Store *)data;
    IndexWriter *iw;
    IndexReader *ir;
    Document **docs = prep_book_list();
    Config config = default_config;
    config.max_buffered_docs = 10;

    iw = create_book_iw_conf(store, &config);
    for (i = 0; i < BOOK_LIST_LENGTH; i++) {
        iw_add_doc(iw, docs[i]);
    }

    /* close */
    iw_close(iw);

    gen = sis_current_segment_generation(store);
    /* segment generation should be > 1 */
    Atrue(gen > 1);

    Atrue(store->exists(store, "_0.cfs"));
    store->remove(store, "_0.cfs");

    TRY
        ir = ir_open(store);
        ir_close(ir);
        Afail("reader should have failed to open on a crashed index");
        break;
    case IO_ERROR:
        HANDLED();
    XCATCHALL
        Afail("reader should have raised an IO_ERROR");
        HANDLED();
    XENDTRY
    destroy_docs(docs, BOOK_LIST_LENGTH);
}

static void test_iw_add_docs(TestCase *tc, void *data)
{
    int i;
    Config config = default_config;
    Store *store = (Store *)data;
    IndexWriter *iw;
    Document **docs = prep_book_list();
    config.merge_factor = 4;
    config.max_buffered_docs = 3;

    iw = create_book_iw_conf(store, &config);
    for (i = 0; i < BOOK_LIST_LENGTH; i++) {
        iw_add_doc(iw, docs[i]);
    }
    iw_optimize(iw);
    Aiequal(BOOK_LIST_LENGTH, iw_doc_count(iw));
    
    iw_close(iw);
    destroy_docs(docs, BOOK_LIST_LENGTH);
    if (!Aiequal(3, store->count(store))) {
        char *buf = store_to_s(store);
        Tmsg("To many files: %s\n", buf);
        free(buf);
    }
}

void test_iw_add_empty_tv(TestCase *tc, void *data)
{
    Store *store = (Store *)data;
    IndexWriter *iw;
    Document *doc;
    FieldInfos *fis = fis_new(STORE_NO, INDEX_YES, TERM_VECTOR_YES);
    fis_add_field(fis, fi_new(I("no_tv"), STORE_YES, INDEX_YES, TERM_VECTOR_NO));
    index_create(store, fis);
    fis_deref(fis);

    iw = iw_open(store, whitespace_analyzer_new(false), &default_config);
    doc = doc_new();
    doc_add_field(doc, df_add_data(df_new(I("tv1")), ""));
    doc_add_field(doc, df_add_data(df_new(I("tv2")), ""));
    doc_add_field(doc, df_add_data(df_new(I("no_tv")), "one two three"));

    iw_add_doc(iw, doc);
    iw_commit(iw);
    Aiequal(1, iw_doc_count(iw));
    iw_close(iw);
    doc_destroy(doc);
}

static void test_iw_del_terms(TestCase *tc, void *data)
{ 
    int i;
    Config config = default_config;
    Store *store = (Store *)data;
    IndexWriter *iw;
    IndexReader *ir;
    Document **docs = prep_book_list();
    char *terms[3];
    config.merge_factor = 4;
    config.max_buffered_docs = 3;

    iw = create_book_iw_conf(store, &config);
    for (i = 0; i < BOOK_LIST_LENGTH; i++) {
        iw_add_doc(iw, docs[i]);
    }
    Aiequal(BOOK_LIST_LENGTH, iw_doc_count(iw));
    iw_close(iw);
    destroy_docs(docs, BOOK_LIST_LENGTH);

    ir = ir_open(store);
    Aiequal(BOOK_LIST_LENGTH, ir->num_docs(ir));
    Aiequal(BOOK_LIST_LENGTH, ir->max_doc(ir));
    ir_close(ir);

    iw = iw_open(store, whitespace_analyzer_new(false), &config);
    iw_delete_term(iw, title, "State");
    iw_close(iw);

    ir = ir_open(store);
    Aiequal(BOOK_LIST_LENGTH - 1, ir->num_docs(ir));
    Aiequal(BOOK_LIST_LENGTH, ir->max_doc(ir));
    ir_close(ir);

    /* test deleting multiple Terms */
    iw = iw_open(store, whitespace_analyzer_new(false), &config);
    iw_delete_term(iw, title, "The");
    iw_delete_term(iw, title, "Blind");
    terms[0] = "Berger";
    terms[1] = "Middleton";
    terms[2] = "DBC";
    iw_delete_terms(iw, author, terms, 3);
    iw_close(iw);

    ir = ir_open(store);
    Aiequal(BOOK_LIST_LENGTH - 17, ir->num_docs(ir));
    Aiequal(BOOK_LIST_LENGTH, ir->max_doc(ir));
    Atrue(!ir->is_deleted(ir, 0));
    Atrue(ir->is_deleted(ir, 1));
    Atrue(ir->is_deleted(ir, 2));
    Atrue(ir->is_deleted(ir, 3));
    Atrue(ir->is_deleted(ir, 4));
    Atrue(ir->is_deleted(ir, 5));
    Atrue(ir->is_deleted(ir, 6));
    Atrue(!ir->is_deleted(ir, 7));
    Atrue(!ir->is_deleted(ir, 9));
    Atrue(ir->is_deleted(ir, 10));
    Atrue(!ir->is_deleted(ir, 11));
    Atrue(!ir->is_deleted(ir, 16));
    Atrue(ir->is_deleted(ir, 17));
    Atrue(ir->is_deleted(ir, 18));
    Atrue(ir->is_deleted(ir, 21));
    Atrue(ir->is_deleted(ir, 23));
    Atrue(ir->is_deleted(ir, 24));
    Atrue(ir->is_deleted(ir, 28));
    Atrue(ir->is_deleted(ir, 30));
    Atrue(ir->is_deleted(ir, 33));
    Atrue(ir->is_deleted(ir, 35));
    Atrue(ir->is_deleted(ir, 36));
    ir_commit(ir);

    iw = iw_open(store, whitespace_analyzer_new(false), &config);
    iw_optimize(iw);
    iw_close(iw);

    ir_close(ir);

    ir = ir_open(store);
    Aiequal(BOOK_LIST_LENGTH - 17, ir->num_docs(ir));
    Aiequal(BOOK_LIST_LENGTH - 17, ir->max_doc(ir));
    ir_close(ir);
}

/****************************************************************************
 *
 * IndexReader
 *
 ****************************************************************************/

static int segment_reader_type = 0;
static int multi_reader_type = 1;
static int multi_external_reader_type = 2;
static int add_indexes_reader_type = 3;

typedef struct ReaderTestEnvironment {
    Store **stores;
    int store_cnt;
} ReaderTestEnvironment;

static void reader_test_env_destroy(ReaderTestEnvironment *rte)
{
    int i;
    for (i = 0; i < rte->store_cnt; i++) {
        store_deref(rte->stores[i]);
    }
    free(rte->stores);
    free(rte);
}

static IndexReader *reader_test_env_ir_open(ReaderTestEnvironment *rte)
{
    if (rte->store_cnt == 1) {
        return ir_open(rte->stores[0]);
    }
    else {
        IndexReader **sub_readers = ALLOC_N(IndexReader *, rte->store_cnt);
        int i;
        for (i = 0; i < rte->store_cnt; i++) {
            sub_readers[i] = ir_open(rte->stores[i]);
        }
        return (mr_open(sub_readers, rte->store_cnt));
    }
}

static ReaderTestEnvironment *reader_test_env_new(int type)
{
    int i, j;
    Config config = default_config;
    IndexWriter *iw;
    Document **docs = prep_ir_test_docs();
    ReaderTestEnvironment *rte = ALLOC(ReaderTestEnvironment);
    int store_cnt = rte->store_cnt
        = (type >= multi_external_reader_type) ? 64 : 1;
    int doc_cnt = IR_TEST_DOC_CNT / store_cnt;

    rte->stores = ALLOC_N(Store *, store_cnt);
    for (i = 0; i < store_cnt; i++) {
        Store *store = rte->stores[i] = open_ram_store();
        FieldInfos *fis = fis_new(STORE_YES, INDEX_YES,
                                  TERM_VECTOR_WITH_POSITIONS_OFFSETS);
        int start_doc = i * doc_cnt;
        int end_doc = (i + 1) * doc_cnt;
        if (end_doc > IR_TEST_DOC_CNT) {
            end_doc = IR_TEST_DOC_CNT;
        }
        index_create(store, fis);
        fis_deref(fis);
        config.max_buffered_docs = 3;

        iw = iw_open(store, whitespace_analyzer_new(false), &config);

        for (j = start_doc; j < end_doc; j++) {
            int k;
            Document *doc = docs[j];
            /* add fields when needed. This is to make the FieldInfos objects
             * different for multi_external_reader */
            for (k = 0; k < doc->size; k++) {
                DocField *df = doc->fields[k];
                fis = iw->fis;
                if (NULL == fis_get_field(fis, df->name)) {
                    if (author == df->name) {
                        fis_add_field(fis, fi_new(author, STORE_YES, INDEX_YES,
                                  TERM_VECTOR_WITH_POSITIONS));
                    } else if (title == df->name) {
                        fis_add_field(fis, fi_new(title, STORE_YES,
                                                  INDEX_UNTOKENIZED,
                                                  TERM_VECTOR_WITH_OFFSETS));
                    } else if (year == df->name) {
                        fis_add_field(fis, fi_new(year, STORE_YES,
                                                  INDEX_UNTOKENIZED,
                                                  TERM_VECTOR_NO));
                    } else if (text == df->name) {
                        fis_add_field(fis, fi_new(text, STORE_NO, INDEX_YES,
                                                  TERM_VECTOR_NO));
                    } else if (compressed_field == df->name) {
                        fis_add_field(fis, fi_new(compressed_field,
                                                  STORE_COMPRESS,
                                                  INDEX_YES,
                                                  TERM_VECTOR_NO));
                    }
                }
            }
            iw_add_doc(iw, doc);
        }

        if (type == segment_reader_type) {
            iw_optimize(iw);
        }
        iw_close(iw);
    }

    if (type == add_indexes_reader_type) {
        /* Prepare store for Add Indexes test */
        Store *store = open_ram_store();
        FieldInfos *fis = fis_new(STORE_YES, INDEX_YES,
                                  TERM_VECTOR_WITH_POSITIONS_OFFSETS);
        IndexReader **readers = ALLOC_N(IndexReader *, rte->store_cnt);
        int i;
        for (i = 0; i < rte->store_cnt; i++) {
            readers[i] = ir_open(rte->stores[i]);
        }
        index_create(store, fis);
        fis_deref(fis);
        iw = iw_open(store, whitespace_analyzer_new(false), &config);
        iw_add_readers(iw, readers, rte->store_cnt - 10);
        iw_close(iw);
        iw = iw_open(store, whitespace_analyzer_new(false), &config);
        iw_add_readers(iw, readers + (rte->store_cnt - 10), 10);
        iw_close(iw);
        for (i = 0; i < rte->store_cnt; i++) {
            ir_close(readers[i]);
            store_deref(rte->stores[i]);
        }
        free(readers);
        rte->stores[0] = store;
        rte->store_cnt = 1;
    }

    destroy_docs(docs, IR_TEST_DOC_CNT);
    return rte;
}

static void write_ir_test_docs(Store *store)
{
    int i;
    Config config = default_config;
    IndexWriter *iw;
    Document **docs = prep_ir_test_docs();

    FieldInfos *fis = fis_new(STORE_YES, INDEX_YES,
                              TERM_VECTOR_WITH_POSITIONS_OFFSETS);
    fis_add_field(fis, fi_new(author, STORE_YES, INDEX_YES,
                              TERM_VECTOR_WITH_POSITIONS));
    fis_add_field(fis, fi_new(title, STORE_YES, INDEX_UNTOKENIZED,
                              TERM_VECTOR_WITH_OFFSETS));
    fis_add_field(fis, fi_new(year, STORE_YES, INDEX_UNTOKENIZED,
                              TERM_VECTOR_NO));
    fis_add_field(fis, fi_new(text, STORE_NO, INDEX_YES,
                              TERM_VECTOR_NO));
    fis_add_field(fis, fi_new(compressed_field, STORE_COMPRESS, INDEX_YES,
                              TERM_VECTOR_NO));
    index_create(store, fis);
    fis_deref(fis);
    config.max_buffered_docs = 5;

    iw = iw_open(store, whitespace_analyzer_new(false), &config);

    for (i = 0; i < IR_TEST_DOC_CNT; i++) {
        iw_add_doc(iw, docs[i]);
    }
    iw_close(iw);

    destroy_docs(docs, IR_TEST_DOC_CNT);
}

static void test_ir_open_empty_index(TestCase *tc, void *data)
{
    Store *store = (Store *)data;
    store->clear_all(store);
    TRY
        ir_close(ir_open(store));
        Afail("IndexReader should have failed when opening empty index");
        break;
    case FILE_NOT_FOUND_ERROR:
        HANDLED();
        break;
    default:
        Afail("IndexReader should have raised FileNotfound Exception");
        HANDLED();
    XENDTRY
}

static void test_ir_basic_ops(TestCase *tc, void *data)
{
    IndexReader *ir = (IndexReader *)data;

    Aiequal(IR_TEST_DOC_CNT, ir->num_docs(ir));
    Aiequal(IR_TEST_DOC_CNT, ir->max_doc(ir));

    Aiequal(4, ir->doc_freq(ir, fis_get_field(ir->fis, body)->number, "Wally"));
    Atrue(ir_is_latest(ir));
}

static void test_ir_term_docpos_enum_skip_to(TestCase *tc,
                                             TermDocEnum *tde,
                                             int field_num)
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

static void test_ir_term_enum(TestCase *tc, void *data)
{
    IndexReader *ir = (IndexReader *)data;
    TermEnum *te = ir_terms(ir, author);

    Asequal("Leo", te->next(te));
    Asequal("Leo", te->curr_term);
    Aiequal(1, te->curr_ti.doc_freq);
    Asequal("Tolstoy", te->next(te));
    Asequal("Tolstoy", te->curr_term);
    Aiequal(1, te->curr_ti.doc_freq);
    Apnull(te->next(te));

    te->set_field(te, fis_get_field_num(ir->fis, body));
    Asequal("And", te->next(te));
    Asequal("And", te->curr_term);
    Aiequal(1, te->curr_ti.doc_freq);

    Asequal("Not", te->skip_to(te, "Not"));
    Asequal("Not", te->curr_term);
    Aiequal(1, te->curr_ti.doc_freq);
    Asequal("Random", te->next(te));
    Asequal("Random", te->curr_term);
    Aiequal(16, te->curr_ti.doc_freq);

    te->set_field(te, fis_get_field_num(ir->fis, text));
    Asequal("which", te->skip_to(te, "which"));
    Asequal("which", te->curr_term);
    Aiequal(1, te->curr_ti.doc_freq);
    Apnull(te->next(te));

    te->set_field(te, fis_get_field_num(ir->fis, title));
    Asequal("Shawshank Redemption", te->next(te));
    Asequal("Shawshank Redemption", te->curr_term);
    Aiequal(1, te->curr_ti.doc_freq);
    Asequal("War And Peace", te->next(te));
    Asequal("War And Peace", te->curr_term);
    Aiequal(1, te->curr_ti.doc_freq);
    te->close(te);

    te = ir_terms_from(ir, body, "No");
    Asequal("Not", te->curr_term);
    Aiequal(1, te->curr_ti.doc_freq);
    Asequal("Random", te->next(te));
    Asequal("Random", te->curr_term);
    Aiequal(16, te->curr_ti.doc_freq);
    te->close(te);
}

static void test_ir_term_doc_enum(TestCase *tc, void *data)
{
    IndexReader *ir = (IndexReader *)data;

    TermDocEnum *tde;
    Document *doc = ir_get_doc_with_term(ir, tag, "id_test");
    int docs[3], expected_docs[3];
    int freqs[3], expected_freqs[3];

    /*
    printf("%s\n", doc_to_s(ir->get_doc(ir, 2)));
    printf("%s\n", doc_to_s(ir->get_doc(ir, 1)));
    printf("%s\n", doc_to_s(ir->get_doc(ir, 0)));
    */
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
    Aiequal(-1, tde->next_position(tde));

    Atrue(tde->next(tde));
    Aiequal(1, tde->doc_num(tde));
    Aiequal(1, tde->freq(tde));
    Aiequal(3, tde->next_position(tde));
    Aiequal(-1, tde->next_position(tde));

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

static void test_ir_term_vectors(TestCase *tc, void *data)
{ 
    IndexReader *ir = (IndexReader *)data;

    TermVector *tv = ir->term_vector(ir, 3, I("body"));
    Hash *tvs;

    Asequal("body", tv->field);
    Aiequal(4, tv->term_cnt);
    Asequal("word1", tv->terms[0].text);
    Asequal("word2", tv->terms[1].text);
    Asequal("word3", tv->terms[2].text);
    Asequal("word4", tv->terms[3].text);
    Aiequal(3, tv->terms[0].freq);
    Aiequal(2, tv->terms[0].positions[0]);
    Aiequal(4, tv->terms[0].positions[1]);
    Aiequal(7, tv->terms[0].positions[2]);
    Aiequal(12, tv->offsets[tv->terms[0].positions[0]].start);
    Aiequal(17, tv->offsets[tv->terms[0].positions[0]].end);
    Aiequal(24, tv->offsets[tv->terms[0].positions[1]].start);
    Aiequal(29, tv->offsets[tv->terms[0].positions[1]].end);
    Aiequal(42, tv->offsets[tv->terms[0].positions[2]].start);
    Aiequal(47, tv->offsets[tv->terms[0].positions[2]].end);

    Aiequal(1, tv->terms[1].freq);
    Aiequal(3, tv->terms[1].positions[0]);
    Aiequal(18, tv->offsets[tv->terms[1].positions[0]].start);
    Aiequal(23, tv->offsets[tv->terms[1].positions[0]].end);

    Aiequal(4, tv->terms[2].freq);
    Aiequal(0, tv->terms[2].positions[0]);
    Aiequal(5, tv->terms[2].positions[1]);
    Aiequal(8, tv->terms[2].positions[2]);
    Aiequal(9, tv->terms[2].positions[3]);
    Aiequal(0,  tv->offsets[tv->terms[2].positions[0]].start);
    Aiequal(5,  tv->offsets[tv->terms[2].positions[0]].end);
    Aiequal(30, tv->offsets[tv->terms[2].positions[1]].start);
    Aiequal(35, tv->offsets[tv->terms[2].positions[1]].end);
    Aiequal(48, tv->offsets[tv->terms[2].positions[2]].start);
    Aiequal(53, tv->offsets[tv->terms[2].positions[2]].end);
    Aiequal(54, tv->offsets[tv->terms[2].positions[3]].start);
    Aiequal(59, tv->offsets[tv->terms[2].positions[3]].end);

    Aiequal(2, tv->terms[3].freq);
    Aiequal(1, tv->terms[3].positions[0]);
    Aiequal(6, tv->terms[3].positions[1]);
    Aiequal(6,  tv->offsets[tv->terms[3].positions[0]].start);
    Aiequal(11, tv->offsets[tv->terms[3].positions[0]].end);
    Aiequal(36, tv->offsets[tv->terms[3].positions[1]].start);
    Aiequal(41, tv->offsets[tv->terms[3].positions[1]].end);

    tv_destroy(tv);

    tvs = ir->term_vectors(ir, 3);
    Aiequal(3, tvs->size);
    tv = (TermVector *)h_get(tvs, I("author"));
    if (Apnotnull(tv)) {
        Asequal("author", tv->field);
        Aiequal(2, tv->term_cnt);
        Aiequal(0, tv->offset_cnt);
        Apnull(tv->offsets);
    }
    tv = (TermVector *)h_get(tvs, I("body"));
    if (Apnotnull(tv)) {
        Asequal("body", tv->field);
        Aiequal(4, tv->term_cnt);
    }
    tv = (TermVector *)h_get(tvs, I("title"));
    if (Apnotnull(tv)) {
        Asequal("title", tv->field);
        Aiequal(1, tv->term_cnt); /* untokenized */
        Aiequal(1, tv->offset_cnt);
        Asequal("War And Peace", tv->terms[0].text);
        Apnull(tv->terms[0].positions);
        Aiequal(0,  tv->offsets[0].start);
        Aiequal(13, tv->offsets[0].end);
    }
    h_destroy(tvs);
}

static void test_ir_get_doc(TestCase *tc, void *data)
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

static void test_ir_compression(TestCase *tc, void *data)
{ 
    int i;
    IndexReader *ir = (IndexReader *)data;
    LazyDoc *lz_doc;
    LazyDocField *lz_df1, *lz_df2;
    Document *doc = ir->get_doc(ir, 0);
    DocField *df1, *df2;
    char buf1[20], buf2[20];
    Aiequal(3, doc->size);

    df1 = doc_get_field(doc, changing_field);
    df2 = doc_get_field(doc, compressed_field);
    Asequal(df1->data[0], df2->data[0]);
    Assert(df1->lengths[0] == df2->lengths[0], "Field lengths should be equal");
    doc_destroy(doc);

    doc = ir->get_doc(ir, 2);
    df1 = doc_get_field(doc, tag);
    df2 = doc_get_field(doc, compressed_field);
    for (i = 0; i < 4; i++) {
        Asequal(df1->data[i], df2->data[i]);
        Assert(df1->lengths[i] == df2->lengths[i], "Field lengths not equal");
    }
    doc_destroy(doc);

    lz_doc = ir->get_lazy_doc(ir, 0);
    lz_df1 = lazy_doc_get(lz_doc, changing_field);
    lz_df2 = lazy_doc_get(lz_doc, compressed_field);
    Asequal(lazy_df_get_data(lz_df1, 0), lazy_df_get_data(lz_df2, 0));
    lazy_doc_close(lz_doc);

    lz_doc = ir->get_lazy_doc(ir, 2);
    lz_df1 = lazy_doc_get(lz_doc, tag);
    lz_df2 = lazy_doc_get(lz_doc, compressed_field);
    for (i = 0; i < 4; i++) {
        Asequal(lazy_df_get_data(lz_df1, i), lazy_df_get_data(lz_df2, i));
    }
    lazy_doc_close(lz_doc);

    lz_doc = ir->get_lazy_doc(ir, 2);
    lz_df1 = lazy_doc_get(lz_doc, tag);
    lz_df2 = lazy_doc_get(lz_doc, compressed_field);
    lazy_df_get_bytes(lz_df1, buf1, 5, 11);
    lazy_df_get_bytes(lz_df2, buf2, 5, 11);
    buf2[11] = buf1[11] = '\0';
    //printf("Read in buf1 <%s>\n", buf1);
    //printf("Read in buf2 <%s>\n", buf2);
    Asequal(buf1, buf2);
    lazy_doc_close(lz_doc);
}

static void test_ir_mtdpe(TestCase *tc, void *data)
{ 
    IndexReader *ir = (IndexReader *)data;
    char *terms[3] = {"Where", "is", "books."};

    TermDocEnum *tde = mtdpe_new(ir, fis_get_field(ir->fis, body)->number,
                                 terms, 3);

    Atrue(tde->next(tde));
    Aiequal(0, tde->doc_num(tde));
    Aiequal(2, tde->freq(tde));
    Aiequal(0, tde->next_position(tde));
    Aiequal(1, tde->next_position(tde));
    Atrue(tde->next(tde));
    Aiequal(20, tde->doc_num(tde));
    Aiequal(2, tde->freq(tde));
    Aiequal(1, tde->next_position(tde));
    Aiequal(17, tde->next_position(tde));
    Atrue(!tde->next(tde));
    tde->close(tde);
}

static void test_ir_norms(TestCase *tc, void *data)
{ 
    int i;
    uchar *norms;
    IndexReader *ir, *ir2;
    IndexWriter *iw;
    int type = *((int *)data);
    ReaderTestEnvironment *rte;

    rte = reader_test_env_new(type);
    ir = reader_test_env_ir_open(rte);
    ir2 = reader_test_env_ir_open(rte);
    Atrue(!index_is_locked(rte->stores[0]));

    ir_set_norm(ir, 3, title, 1);
    Atrue(index_is_locked(rte->stores[0]));
    ir_set_norm(ir, 3, body, 12);
    ir_set_norm(ir, 3, author, 145);
    ir_set_norm(ir, 3, year, 31);
    ir_set_norm(ir, 5, text, 202);
    ir_set_norm(ir, 25, text, 20);
    ir_set_norm(ir, 50, text, 200);
    ir_set_norm(ir, 75, text, 155);
    ir_set_norm(ir, 80, text, 0);
    ir_set_norm(ir, 150, text, 255);
    ir_set_norm(ir, 255, text, 76);

    ir_commit(ir);
    Atrue(!index_is_locked(rte->stores[0]));

    norms = ir_get_norms(ir, text);

    Aiequal(202, norms[5]);
    Aiequal(20, norms[25]);
    Aiequal(200, norms[50]);
    Aiequal(155, norms[75]);
    Aiequal(0, norms[80]);
    Aiequal(255, norms[150]);
    Aiequal(76, norms[255]);

    norms = ir_get_norms(ir, title);
    Aiequal(1, norms[3]);

    norms = ir_get_norms(ir, body);
    Aiequal(12, norms[3]);

    norms = ir_get_norms(ir, author);
    Aiequal(145, norms[3]);

    norms = ir_get_norms(ir, year);
    /* Apnull(norms); */

    norms = ALLOC_N(uchar, 356);
    ir_get_norms_into(ir, text, norms + 100);
    Aiequal(202, norms[105]);
    Aiequal(20, norms[125]);
    Aiequal(200, norms[150]);
    Aiequal(155, norms[175]);
    Aiequal(0, norms[180]);
    Aiequal(255, norms[250]);
    Aiequal(76, norms[355]);

    ir_commit(ir);

    for (i = 0; i < rte->store_cnt; i++) {
        iw = iw_open(rte->stores[i], whitespace_analyzer_new(false),
                     &default_config);
        iw_optimize(iw);
        iw_close(iw);
    }

    ir_close(ir);

    ir = reader_test_env_ir_open(rte);

    memset(norms, 0, 356);
    ir_get_norms_into(ir, text, norms + 100);
    Aiequal(0, norms[102]);
    Aiequal(202, norms[105]);
    Aiequal(0, norms[104]);
    Aiequal(20, norms[125]);
    Aiequal(200, norms[150]);
    Aiequal(155, norms[175]);
    Aiequal(0, norms[180]);
    Aiequal(255, norms[250]);
    Aiequal(76, norms[355]);

    Atrue(!index_is_locked(rte->stores[0]));
    ir_set_norm(ir, 0, text, 155);
    Atrue(index_is_locked(rte->stores[0]));
    ir_close(ir);
    ir_close(ir2);
    Atrue(!index_is_locked(rte->stores[0]));
    reader_test_env_destroy(rte);
    free(norms);
}

static void test_ir_delete(TestCase *tc, void *data)
{
    int i;
    Store *store = open_ram_store();
    IndexReader *ir, *ir2;
    IndexWriter *iw;
    int type = *((int *)data);
    ReaderTestEnvironment *rte;

    rte = reader_test_env_new(type);
    ir = reader_test_env_ir_open(rte);
    ir2 = reader_test_env_ir_open(rte);

    Aiequal(false, ir->has_deletions(ir));
    Aiequal(IR_TEST_DOC_CNT, ir->max_doc(ir));
    Aiequal(IR_TEST_DOC_CNT, ir->num_docs(ir));
    Aiequal(false, ir->is_deleted(ir, 10));

    ir_delete_doc(ir, 10);
    Aiequal(true, ir->has_deletions(ir));
    Aiequal(IR_TEST_DOC_CNT, ir->max_doc(ir));
    Aiequal(IR_TEST_DOC_CNT - 1, ir->num_docs(ir));
    Aiequal(true, ir->is_deleted(ir, 10));

    ir_delete_doc(ir, 10);
    Aiequal(true, ir->has_deletions(ir));
    Aiequal(IR_TEST_DOC_CNT, ir->max_doc(ir));
    Aiequal(IR_TEST_DOC_CNT - 1, ir->num_docs(ir));
    Aiequal(true, ir->is_deleted(ir, 10));

    ir_delete_doc(ir, IR_TEST_DOC_CNT - 1);
    Aiequal(true, ir->has_deletions(ir));
    Aiequal(IR_TEST_DOC_CNT, ir->max_doc(ir));
    Aiequal(IR_TEST_DOC_CNT - 2, ir->num_docs(ir));
    Aiequal(true, ir->is_deleted(ir, IR_TEST_DOC_CNT - 1));

    ir_delete_doc(ir, IR_TEST_DOC_CNT - 2);
    Aiequal(true, ir->has_deletions(ir));
    Aiequal(IR_TEST_DOC_CNT, ir->max_doc(ir));
    Aiequal(IR_TEST_DOC_CNT - 3, ir->num_docs(ir));
    Aiequal(true, ir->is_deleted(ir, IR_TEST_DOC_CNT - 2));

    ir_undelete_all(ir);
    Aiequal(false, ir->has_deletions(ir));
    Aiequal(IR_TEST_DOC_CNT, ir->max_doc(ir));
    Aiequal(IR_TEST_DOC_CNT, ir->num_docs(ir));
    Aiequal(false, ir->is_deleted(ir, 10));
    Aiequal(false, ir->is_deleted(ir, IR_TEST_DOC_CNT - 2));
    Aiequal(false, ir->is_deleted(ir, IR_TEST_DOC_CNT - 1));

    ir_delete_doc(ir, 10);
    ir_delete_doc(ir, 20);
    ir_delete_doc(ir, 30);
    ir_delete_doc(ir, 40);
    ir_delete_doc(ir, 50);
    ir_delete_doc(ir, IR_TEST_DOC_CNT - 1);
    Aiequal(true, ir->has_deletions(ir));
    Aiequal(IR_TEST_DOC_CNT, ir->max_doc(ir));
    Aiequal(IR_TEST_DOC_CNT - 6, ir->num_docs(ir));

    ir_close(ir);

    ir = reader_test_env_ir_open(rte);

    Aiequal(true, ir->has_deletions(ir));
    Aiequal(IR_TEST_DOC_CNT, ir->max_doc(ir));
    Aiequal(IR_TEST_DOC_CNT - 6, ir->num_docs(ir));
    Aiequal(true, ir->is_deleted(ir, 10));
    Aiequal(true, ir->is_deleted(ir, 20));
    Aiequal(true, ir->is_deleted(ir, 30));
    Aiequal(true, ir->is_deleted(ir, 40));
    Aiequal(true, ir->is_deleted(ir, 50));
    Aiequal(true, ir->is_deleted(ir, IR_TEST_DOC_CNT - 1));

    ir_undelete_all(ir);
    Aiequal(false, ir->has_deletions(ir));
    Aiequal(IR_TEST_DOC_CNT, ir->max_doc(ir));
    Aiequal(IR_TEST_DOC_CNT, ir->num_docs(ir));
    Aiequal(false, ir->is_deleted(ir, 10));
    Aiequal(false, ir->is_deleted(ir, 20));
    Aiequal(false, ir->is_deleted(ir, 30));
    Aiequal(false, ir->is_deleted(ir, 40));
    Aiequal(false, ir->is_deleted(ir, 50));
    Aiequal(false, ir->is_deleted(ir, IR_TEST_DOC_CNT - 1));

    ir_delete_doc(ir, 10);
    ir_delete_doc(ir, 20);
    ir_delete_doc(ir, 30);
    ir_delete_doc(ir, 40);
    ir_delete_doc(ir, 50);
    ir_delete_doc(ir, IR_TEST_DOC_CNT - 1);

    ir_commit(ir);

    for (i = 0; i < rte->store_cnt; i++) {
        iw = iw_open(rte->stores[i], whitespace_analyzer_new(false),
                     &default_config);
        iw_optimize(iw);
        iw_close(iw);
    }

    ir_close(ir);
    ir = reader_test_env_ir_open(rte);

    Aiequal(false, ir->has_deletions(ir));
    Aiequal(IR_TEST_DOC_CNT - 6, ir->max_doc(ir));
    Aiequal(IR_TEST_DOC_CNT - 6, ir->num_docs(ir));

    Atrue(ir_is_latest(ir));
    Atrue(!ir_is_latest(ir2));

    ir_close(ir);
    ir_close(ir2);
    reader_test_env_destroy(rte);
    store_deref(store);
}

static void test_ir_read_while_optimizing(TestCase *tc, void *data)
{
    Store *store = (Store *)data;
    IndexReader *ir;
    IndexWriter *iw;

    write_ir_test_docs(store);

    ir = ir_open(store);

    test_ir_term_doc_enum(tc, ir);

    iw = iw_open(store, whitespace_analyzer_new(false), false);
    iw_optimize(iw);
    iw_close(iw);

    test_ir_term_doc_enum(tc, ir);

    ir_close(ir);
}

static void test_ir_multivalue_fields(TestCase *tc, void *data)
{ 
    Store *store = (Store *)data;
    IndexReader *ir;
    FieldInfo *fi;
    Document *doc = doc_new();
    DocField *df;
    IndexWriter *iw;
    FieldInfos *fis = fis_new(STORE_YES, INDEX_YES,
                              TERM_VECTOR_WITH_POSITIONS_OFFSETS);
    char *body_text = "this is the body Document Field";
    char *title_text = "this is the title Document Field";
    char *author_text = "this is the author Document Field";
    
    index_create(store, fis);
    fis_deref(fis);
    iw = iw_open(store, whitespace_analyzer_new(false), NULL);

    df = doc_add_field(doc, df_add_data(df_new(tag), "Ruby"));
    df_add_data(df, "C");
    doc_add_field(doc, df_add_data(df_new(body), body_text));
    df_add_data(df, "Lucene");
    doc_add_field(doc, df_add_data(df_new(title), title_text));
    df_add_data(df, "Ferret");
    doc_add_field(doc, df_add_data(df_new(author), author_text));

    Aiequal(0, iw->fis->size);

    iw_add_doc(iw, doc);

    fi = fis_get_field(iw->fis, tag);
    Aiequal(true, fi_is_stored(fi));
    Aiequal(false, fi_is_compressed(fi));
    Aiequal(true, fi_is_indexed(fi));
    Aiequal(true, fi_is_tokenized(fi));
    Aiequal(true, fi_has_norms(fi));
    Aiequal(true, fi_store_term_vector(fi));
    Aiequal(true, fi_store_offsets(fi));
    Aiequal(true, fi_store_positions(fi));

    doc_destroy(doc);
    iw_close(iw);

    ir = ir_open(store);

    doc = ir->get_doc(ir, 0);
    Aiequal(4, doc->size);
    df = doc_get_field(doc, tag);
    Aiequal(4, df->size);
    Asequal("Ruby",   df->data[0]);
    Asequal("C",      df->data[1]);
    Asequal("Lucene", df->data[2]);
    Asequal("Ferret", df->data[3]);

    df = doc_get_field(doc, body);
    Aiequal(1, df->size);
    Asequal(body_text, df->data[0]);

    df = doc_get_field(doc, title);
    Aiequal(1, df->size);
    Asequal(title_text, df->data[0]);

    df = doc_get_field(doc, author);
    Aiequal(1, df->size);
    Asequal(author_text, df->data[0]);

    doc_destroy(doc);
    ir_delete_doc(ir, 0);
    ir_close(ir);
}

/***************************************************************************
 *
 * IndexSuite
 *
 ***************************************************************************/
TestSuite *ts_index(TestSuite *suite)
{
    IndexReader *ir;
    Store *fs_store, *store = open_ram_store();
    ReaderTestEnvironment *rte = NULL;
    /* Store *store = open_fs_store(TEST_DIR); */

    /* initialize Symbols */
    body             = intern("body");
    title            = intern("title");
    text             = intern("text");
    author           = intern("author");
    year             = intern("year");
    changing_field   = intern("changing_field");
    compressed_field = intern("compressed_field");
    tag              = intern("tag");

    srand(5);
    suite = tst_add_suite(suite, "test_term_doc_enum");

    /* TermDocEnum */
    tst_run_test(suite, test_segment_term_doc_enum, store);
    tst_run_test(suite, test_segment_tde_deleted_docs, store);

    suite = ADD_SUITE(suite);
    /* Index */
    tst_run_test(suite, test_index_create, store);
    tst_run_test(suite, test_index_version, store);
    tst_run_test(suite, test_index_undelete_all_after_close, store);

    /* IndexWriter */
    tst_run_test(suite, test_fld_inverter, store);
    tst_run_test(suite, test_postings_sorter, NULL);
    tst_run_test(suite, test_iw_add_doc, store);
    tst_run_test(suite, test_iw_add_docs, store);
    tst_run_test(suite, test_iw_add_empty_tv, store);
    tst_run_test(suite, test_iw_del_terms, store);
    tst_run_test(suite, test_create_with_reader, store);
    tst_run_test(suite, test_simulated_crashed_writer, store);
    tst_run_test(suite, test_simulated_corrupt_index1, store);
    tst_run_test(suite, test_simulated_corrupt_index2, store);

    /* IndexReader */
    tst_run_test(suite, test_ir_open_empty_index, store);

    /* Test SEGMENT Reader */
    rte = reader_test_env_new(segment_reader_type);
    ir = reader_test_env_ir_open(rte);

    tst_run_test_with_name(suite, test_ir_basic_ops, ir,
                           "test_segment_reader_basic_ops");
    tst_run_test_with_name(suite, test_ir_get_doc, ir,
                           "test_segment_get_doc");
    tst_run_test_with_name(suite, test_ir_compression, ir,
                           "test_segment_compression");
    tst_run_test_with_name(suite, test_ir_term_enum, ir,
                           "test_segment_term_enum");
    tst_run_test_with_name(suite, test_ir_term_doc_enum, ir,
                           "test_segment_term_doc_enum");
    tst_run_test_with_name(suite, test_ir_term_vectors, ir,
                           "test_segment_term_vectors");
    tst_run_test_with_name(suite, test_ir_mtdpe, ir,
                           "test_segment_multiple_term_doc_pos_enum");

    tst_run_test_with_name(suite, test_ir_norms, &segment_reader_type,
                           "test_segment_norms");
    tst_run_test_with_name(suite, test_ir_delete, &segment_reader_type,
                           "test_segment_reader_delete");
    ir_close(ir);
    reader_test_env_destroy(rte);

    /* Test MULTI Reader */
    rte = reader_test_env_new(multi_reader_type);
    ir = reader_test_env_ir_open(rte);

    tst_run_test_with_name(suite, test_ir_basic_ops, ir,
                           "test_multi_reader_basic_ops");
    tst_run_test_with_name(suite, test_ir_get_doc, ir,
                           "test_multi_get_doc");
    tst_run_test_with_name(suite, test_ir_compression, ir,
                           "test_multi_compression");
    tst_run_test_with_name(suite, test_ir_term_enum, ir,
                           "test_multi_term_enum");
    tst_run_test_with_name(suite, test_ir_term_doc_enum, ir,
                           "test_multi_term_doc_enum");
    tst_run_test_with_name(suite, test_ir_term_vectors, ir,
                           "test_multi_term_vectors");
    tst_run_test_with_name(suite, test_ir_mtdpe, ir,
                           "test_multi_multiple_term_doc_pos_enum");

    tst_run_test_with_name(suite, test_ir_norms, &multi_reader_type,
                           "test_multi_norms");
    tst_run_test_with_name(suite, test_ir_delete, &multi_reader_type,
                           "test_multi_reader_delete");
    ir_close(ir);
    reader_test_env_destroy(rte);

    /* Test MULTI Reader with seperate stores */
    rte = reader_test_env_new(multi_external_reader_type);
    ir = reader_test_env_ir_open(rte);

    tst_run_test_with_name(suite, test_ir_basic_ops, ir,
                           "test_multi_ext_reader_basic_ops");
    tst_run_test_with_name(suite, test_ir_get_doc, ir,
                           "test_multi_ext_get_doc");
    tst_run_test_with_name(suite, test_ir_compression, ir,
                           "test_multi_ext_compression");
    tst_run_test_with_name(suite, test_ir_term_enum, ir,
                           "test_multi_ext_term_enum");
    tst_run_test_with_name(suite, test_ir_term_doc_enum, ir,
                           "test_multi_ext_term_doc_enum");
    tst_run_test_with_name(suite, test_ir_term_vectors, ir,
                           "test_multi_ext_term_vectors");
    tst_run_test_with_name(suite, test_ir_mtdpe, ir,
                           "test_multi_ext_multiple_term_doc_pos_enum");

    tst_run_test_with_name(suite, test_ir_norms, &multi_external_reader_type,
                           "test_multi_ext_norms");
    tst_run_test_with_name(suite, test_ir_delete, &multi_external_reader_type,
                           "test_multi_ext_reader_delete");

    ir_close(ir);
    reader_test_env_destroy(rte);

    /* Test Add Indexes */
    rte = reader_test_env_new(add_indexes_reader_type);
    ir = reader_test_env_ir_open(rte);

    tst_run_test_with_name(suite, test_ir_basic_ops, ir,
                           "test_add_indexes_reader_basic_ops");
    tst_run_test_with_name(suite, test_ir_get_doc, ir,
                           "test_add_indexes_get_doc");
    tst_run_test_with_name(suite, test_ir_compression, ir,
                           "test_add_indexes_compression");
    tst_run_test_with_name(suite, test_ir_term_enum, ir,
                           "test_add_indexes_term_enum");
    tst_run_test_with_name(suite, test_ir_term_doc_enum, ir,
                           "test_add_indexes_term_doc_enum");
    tst_run_test_with_name(suite, test_ir_term_vectors, ir,
                           "test_add_indexes_term_vectors");
    tst_run_test_with_name(suite, test_ir_mtdpe, ir,
                           "test_add_indexes_multiple_term_doc_pos_enum");

    tst_run_test_with_name(suite, test_ir_norms, &add_indexes_reader_type,
                           "test_add_indexes_norms");
    tst_run_test_with_name(suite, test_ir_delete, &add_indexes_reader_type,
                           "test_add_indexes_reader_delete");

    ir_close(ir);
    reader_test_env_destroy(rte);

    /* Other IndexReader Tests */
    tst_run_test_with_name(suite, test_ir_read_while_optimizing, store,
                           "test_ir_read_while_optimizing_in_ram");

    fs_store = open_fs_store(TEST_DIR);
    tst_run_test_with_name(suite, test_ir_read_while_optimizing, fs_store,
                           "test_ir_read_while_optimizing_on_disk");
    fs_store->clear_all(fs_store);
    store_deref(fs_store);

    tst_run_test(suite, test_ir_multivalue_fields, store);

    store_deref(store);
    return suite;
}
