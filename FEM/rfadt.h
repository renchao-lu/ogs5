/**************************************************************************/
/* Header-Datei: rfadt.h                                                  */
/*                                                                        */
/* Aufgabe:                                                               */
/* estellt abstrakte Datentypen mit beliebigen Elementen.                 */
/* adt : Listen, Stacks und Warteschlangen                                */
/*                                                                        */
/**************************************************************************/

#ifndef __RFADT_H

#define __RFADT_H

/*==================== Single List Item ====================*/

/* Datentyp sList_Item */
typedef struct _sList_Item {
  void *member;

  struct _sList_Item *next;
} sList_Item;

/* Datentyp sList */
typedef struct _sList{
  long size;
  long max_size;
  long inc_size;

  struct _sList_Item *start;
  struct _sList_Item *end;
  struct _sList_Item *current;
} sList;


/* item erzeugen */
sList_Item *create_slist_item(void);
/* item loeschen */
sList_Item *destroy_slist_item(sList *slist, sList_Item *item);
/* current-item loeschen */
sList_Item *destroy_slist_item_current(sList *slist);

/* Liste erzeugen */
sList  *create_slist(long size, long inc_size);
/* Liste loeschen */
sList  *destroy_slist(sList *slist);
/* Listengroesse */
long slist_size(sList *slist);
/* Leere Liste */
int slist_empty(sList *slist);
/* Liste leeren */
void delete_slist(sList *slist, void (*_delete_function)(void *));


/* =========  Member-Funktionen  ========= */
/* member in der Liste einfuegen (direction=-1 --> vor  item)
                 (direction= 1 --> nach item) */
sList_Item *put_slist_member(sList *slist, sList_Item *item,
                     int direction,void *member);

/* member am Ende der Liste einfuegen */
sList_Item *append_slist_member(sList *slist, void *member);

/* member am Ende der Liste einfuegen
   Returnwert ist die Position in der Liste (Hier: Size-1) */
long append_slist(sList *slist, void *member);


/*==================== Double List Item ====================*/

/* Datentyp List_Item */
typedef struct _List_Item {
  void *member;

  struct _List_Item *last;
  struct _List_Item *next;
} List_Item;


/* Datentyp List */
typedef struct _List{
  long size;

  struct _List_Item *start;
  struct _List_Item *iterator;
  struct _List_Item *current;
} LList;


/* item erzeugen */
List_Item *create_list_item(void);
/* item loeschen */
List_Item *destroy_list_item(LList *RFlist, List_Item *item);
/* current-item loeschen */
List_Item *destroy_list_item_current(LList *RFlist);



/* Liste erzeugen */
LList  *create_list(void);
/* Liste loeschen */
LList  *destroy_list(LList *RFlist);
/* Listengroesse */
long list_size(LList *RFlist);
/* Leere Liste */
int list_empty(LList *RFlist);
/* Liste leeren */
void delete_list(LList *RFlist, void (*_delete_function)(void *));
/* Liefert niedigsten freien Feldindex */
long get_lowest_free (LList *RFlist);

/* =========  Member-Funktionen  ========= */

/* Item und Member loeschen und aus der Liste entfernen */
void destroy_list_member_pos(LList *RFlist, long pos,
                 void (*_delete_function)(void *));


/* member in der Liste einfuegen (direction=-1 --> vor  item)
                                 (direction= 1 --> nach item) */
List_Item *put_list_member(LList *RFlist, List_Item *item,
                                     int direction,void *member);

/* item aus der Liste holen (Suchen) */
List_Item *get_list_member(LList *RFlist, void *member);


/* Member in der Position pos aus der Liste holen
   ( pos von 0 bis size-1 ) (Suchen) */
void *get_list_member_pos(LList *RFList, long pos);

/* member am Ende der Liste einfuegen */
List_Item *append_list_member(LList *RFList, void *member);

/* member am Ende der Liste einfuegen
   Returnwert ist die Position in der Liste (Hier: Size-1) */
long append_list(LList *RFList, void *member);


/* member am Anfang der Liste einfuegen */
List_Item *insert_list_member(LList *RFList, void *member);

/* new_member hinter member in der Liste einfuegen */
void insert_list_after(LList *RFList, void *member, void *new_member);

/* new_member vor member in der Liste einfuegen */
void insert_list_bevor(LList *RFList, void *member, void *new_member);

/* member an der Position pos in der Liste einfuegen */
void insert_list_pos(LList *RFList, long pos, void *member);

/* member an der Position pos in der Liste eintragen */
void put_list_pos(LList *RFList, long pos, void *member);

/* member aus der Liste entfernen */
void delete_list_member(LList *RFList, void *member);
long remove_list_member(LList *RFList, void *member,void (*_delete_function)(void *));

/* member suchen mit Vergleichsfunktion  (Suchen) */
List_Item *search_list_member(LList *RFList, void *member,
                               int (*_compare_function)(void *, void *));

/* Element suchen mit Vergleichsfunktion  (Suchen) */
void *search_list(LList *RFList, void *member,
                int (*_compare_function)(void *, void *));


/* =========  Item-Funktionen  ========= */

/* Item-Inhalt aus der Liste holen */
void *get_list_item_content(LList *RFList, List_Item *item);

/* Item-Position aus der Liste holen ( von 0 bis size-1 ) (Suchen) */
long get_list_item_pos_list(LList *RFList, List_Item *item);

/* Item in der Position pos aus der Liste holen
   ( pos von 0 bis size-1 ) (Suchen) */
List_Item *get_list_item_pos(LList *RFList, long pos);

/* Erstes Item der Liste holen */
List_Item *list_item_first(LList *RFList);

/* Letztes Item der Liste holen */
List_Item *list_item_last(LList *RFList);

/* Item in der Position pos loeschen ( pos von 0 bis size-1 ) */
void delete_list_item_pos(LList *RFList, long pos);


/* =========  Iterator-Funktionen  ========= */

/* Iterator initialisierren */
void list_iterator_init(LList *RFList);

/* aktueller Iterator */
List_Item *list_iterator_current(LList *RFList);

/* Iterator nach vorne bewegen */
List_Item *list_iterator_next(LList *RFList);

/* Iterator zurueck bewegen */
List_Item *list_iterator_preview(LList *RFList);

/* Iterator am Anfang positionieren */
List_Item *list_iterator_first(LList *RFList);

/* Iterator am Ende positionieren */
List_Item *list_iterator_last(LList *RFList);


/* =========  Current-Funktionen  ========= */

/* current initialisieren */
void list_current_init(LList *RFList);

/* current auf naechstes item bewegen */
void *list_current_next(LList *RFList);

/* current auf vorheriges item bewegen */
void *list_current_preview(LList *RFList);

/* Aktuelles member in der Liste holen */
void *get_list_current(LList *RFList);

/* Naechstes member in der Liste holen */
void *get_list_next(LList *RFList);

/* Vorheriges member in der Liste holen */
void *get_list_preview(LList *RFList);



/* =========  Listen-Funktionen  ========= */

/* Kopfelement der Liste holen */
void *get_list_head(LList *RFList);

/* Schwanzelement der Liste holen */
void *get_list_tail(LList *RFList);



/* =========  Ein- Ausgabe-Funktionen  ========= */

/* Liste ausgeben */
void print_list(LList *RFList, void (*_print_function)(void *));

/* Liste in eine Datei schreiben */
void write_list(LList *RFList, FILE *fp, void (*_write_function)(FILE *, void *));

/* Liste aus einer Datei lesen (noch nicht fertig !!!) */
void load_list(LList *RFList, FILE *fp, void (*_load_function)(FILE *, void *));


/*==================== Stack List ====================*/

/* Datentyp Stack */
typedef struct {
  LList *list;
} Stack;


/* =========  Stack-Funktionen  ========= */

/* Stack erzeugen */
Stack  *create_stack(void);
/* Stackgroesse */
long stack_size(Stack *stack);
/* Leere Liste */
int stack_empty(Stack *RFlist);

/* member aus dem Stack holen */
void *top_stack_member(Stack *stack);
/* member in Stack einfuegen */
void push_stack_member(Stack *stack, void *member);
/* member aus dem Stack entfernen und holen */
void *pop_stack_member(Stack *stack);

/* Stack ausgeben */
void print_stack(Stack *stack, void (*_print_function)(void *));


/*==================== Queue List ====================*/

/* Datentyp Queue */
typedef struct {
  LList *list;
} Queue;



/* =========  Queue-Funktionen  ========= */

/* Queue erzeugen */
Queue  *create_queue(void);
/* Queuegroesse */
long queue_size(Queue *queue);
/* Leere Queue */
int queue_empty(Queue *queue);

/* member aus der Queue holen */
void *front_queue_member(Queue *queue);
/* member in Queue einfuegen */
void put_queue_member(Queue *queue, void *member);
/* member aus der Queue entfernen und holen */
void *get_queue_member(Queue *queue);

/* Queue ausgeben */
void print_queue(Queue *queue, void (*_print_function)(void *));


/*==================== Binary tree List ====================*/


/* Datentyp BinTree_Item */
typedef struct _BinTree_Item {
  void *member;

  struct _BinTree_Item *left;
  struct _BinTree_Item *right;
} BinTree_Item;


/* Datentyp BinTree */
typedef struct _BinTree{

  struct _BinTree_Item *root;
  long size;
} BinTree;



/* =========  BinTree-Funktionen  ========= */

/* BinTree_Item erzeugen */
BinTree_Item *create_bintree_item(void);
/* BinTree_Item loeschen (noch nicht fertig !!!) */
BinTree_Item *destroy_bintree_item(BinTree *bintree, BinTree_Item *item);


/* BinTree erzeugen */
BinTree  *create_bintree(void);
/* BinTree zerstoeren */
BinTree  *destroy_bintree(BinTree *bintree);
/* BinTree-Groesse */
long bintree_size(BinTree *bintree);
/* Leere BinTree */
int bintree_empty(BinTree *bintree);

/*neu*/
/* member in BinTree einfuegen */
void append_bintree_member_V0(BinTree *bintree, void *member,
                           int (*_compare_function)(void *, void *));
/* member in BinTree einfuegen mit Identifikator-Funktion */
void append_bintree_member(BinTree *bintree, void *member,
                           long (*_id_function)(void *));

/* member aus der Liste entfernen (rekursiv) (noch nicht fertig !!!)*/
void delete_bintree_member_r(BinTree *bintree, void *member);
/* member aus der Liste entfernen (iterativ) (noch nicht fertig !!!)*/
void delete_bintree_member(BinTree *bintree, void *member,
                               int (*_compare_function)(void *, void *));
/* BinTree_Item Ým Baum suchen (Suchen) */
BinTree_Item *get_bintree_member(BinTree *bintree, BinTree_Item *item,
                                                                 void *member);
/*neu*/
/* BinTree_Item im Baum einfuegen */
BinTree_Item *append_bintree_item_V0(BinTree_Item *item1, BinTree_Item *item2,
                               int (*_compare_function)(void *, void *));

/* BinTree_Item im Baum einfuegen mit Identifikator-Funktion */
BinTree_Item *append_bintree_item(BinTree_Item *item1, BinTree_Item *item2,
                               long (*_id_function)(void *));



/* BinTree durchsuchen */
BinTree_Item* search_bintree(BinTree *bintree, void *member,
                    int (*_compare_function)(void *, void *));
/* BinTree_Item suchen (rekursiv) */
BinTree_Item *search_bintree_item_r(BinTree_Item *item, void *member,
                    int (*_compare_function)(void *, void *));
/* BinTree_Item suchen */
BinTree_Item **search_bintree_item(BinTree_Item **item, void *member,
                    int (*_compare_function)(void *, void *));



/* Neue Funktionen */


/* BinTree_Item loeschen (iterativ) */
void destroy_bintree_item_id(BinTree_Item **item,
                    void (*_delete_function)(void *));
/* Item-Inhalt aus der Baumstruktur holen */
void *get_bintree_item_content(BinTree *bintree, BinTree_Item *item);
/* BinTree_Item suchen (iterativ) mit Identifikator-Funktion */
BinTree_Item **search_bintree_item_id(BinTree_Item **item, long id,
                    long (*_id_function)(void *));
/* member suchen (iterativ) mit Identifikator-Funktion */
void *search_bintree_member_id(BinTree *bintree, long id,
                    long (*_id_function)(void *));
/* member suchen (iterativ) mit Identifikator-Funktion */
void *delete_bintree_member_id(BinTree *bintree, long id,
                    long (*_id_function)(void *),
                    void (*_delete_function)(void *));
/* Ast loeschen mit Identifikator-Funktion */
void *delete_bintree_id(BinTree *bintree, long id,
                    long (*_id_function)(void *),
                    void (*_delete_function)(void *));
/* Ast von BinTree_Item loeschen */
void destroy_bintree_id(BinTree_Item **item,
                    void (*_delete_function)(void *));
/* member in BinTree einfuegen (iterativ) */
int insert_bintree_member(BinTree *bintree, void *member,
                           long (*_id_function)(void *));



/* BinTree ausgeben */
void print_bintree(BinTree *bintree, void (*_print_function)(void *));
/* BinTree_Item ausgeben */
void print_bintree_item(BinTree_Item *item,
                        void (*_print_function)(void *));



#endif   /* __RFADT_H */
