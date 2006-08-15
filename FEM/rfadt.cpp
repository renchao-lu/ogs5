#include "stdafx.h" /* MFC */


#include <stdio.h>
#include <stdlib.h>


/* Preprozessor-Definitionen */
#include "rfadt.h"


/* =========  sList_Item : Konstruktor und Destruktor  ========= */

/* -------------------------------------------------------------- */
/* item erzeugen
                                  */
/* -------------------------------------------------------------- */
sList_Item *create_slist_item(void)
{
  sList_Item *item;

  item = (sList_Item *) malloc(sizeof(sList_Item));

  if ( item == NULL ) {
     printf("\nNicht genug Speicher um einen neuen Listen-Item zu erstellen (create_slist_item)\n");
  }

  return item;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* item loeschen
                                  */
/* -------------------------------------------------------------- */
sList_Item *destroy_slist_item(sList *slist, sList_Item *item)
{
  sList_Item *q;

  if ( item == NULL ) {
     printf("\nItem ist ein Nullzeiger (destroy_list_item)\n");
  }
  else if ( item == slist->end ) {
     printf("\nSentinel kann nicht geloescht werden (sdestroy_list_item) !!!\n");
  }
  else {
    q=item->next;
    if (q == slist->end) slist->end = item;
    else *item=*q;
    free(q);
  }

  return NULL;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* current-item loeschen
                                  */
/* -------------------------------------------------------------- */
sList_Item *destroy_slist_item_current(sList *slist)
{
  return destroy_slist_item(slist,slist->current);
}
/* -------------------------------------------------------------- */


/* =========  sList : Konstruktor, Destruktor usw. ========= */

/* -------------------------------------------------------------- */
/* Liste: Konstruktor
                                  */
/* -------------------------------------------------------------- */
sList  *create_slist(long size, long inc_size)
{
  sList *slist;
  sList_Item *p;
  long i;

  slist = (sList *) malloc(sizeof(sList));

  if ( slist == NULL ) {
     printf("\nNicht genug Speicher um eine neue Liste zu erstellen\n");
     return NULL;
  }
  slist->size=0;
  slist->start = create_slist_item();       /* Sentinel erzeugen */
  slist->end = slist->start;

  if(size > 0) {
    for(i=0; i<size; i++) {   /* Liste mit Elementengroesse size erzeugen */
      p=slist->start;
      slist->start = create_slist_item();
      slist->start->next=p;
    }
    slist->max_size=size;
  }

  slist->current = slist->start;
  if(inc_size > 0) slist->inc_size=inc_size;
  else             slist->inc_size=1;

  return slist;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* Liste: Konstruktor (fast mode)
                                  */
/* -------------------------------------------------------------- */
sList  *create_slist_fm(long size, long inc_size)
{
  sList *slist;
  sList_Item *p;
  long i;
  sList_Item **q;

  slist = (sList *) malloc(sizeof(sList));

  if ( slist == NULL ) {
     printf("\nNicht genug Speicher um eine neue Liste zu erstelle (create_slist_fm)\n");
     return NULL;
  }
  slist->size=0;
  slist->start = (sList_Item *) malloc(sizeof(sList_Item));  /* Sentinel erzeugen */
  if ( slist->start == NULL ) {
     printf("\nNicht genug Speicher um einen neuen Listen-Item zu erstellen (create_slist_fm)\n");
  }
  slist->end = slist->start;

  if(size > 0) {
    q = (sList_Item **) malloc(size*sizeof(sList_Item));
    if ( q == NULL ) {
      printf("\nNicht genug Speicher um einen neuen Listen-Item zu erstellen (create_slist_fm)\n");
    }
    for(i=0; i<size; i++) {   /* Liste mit Elementengroesse size erzeugen */
      p=slist->start;
      slist->start = q[i];
      slist->start->next=p;
    }
    slist->max_size=size;
  }

  slist->current = slist->start;
  if(inc_size > 0) slist->inc_size=inc_size;
  else             slist->inc_size=1;

  return slist;
}
/* -------------------------------------------------------------- */

/* -------------------------------------------------------------- */
/* Liste: Destruktor
                                  */
/* -------------------------------------------------------------- */
sList  *destroy_slist(sList *slist)
{
  if ( !slist_empty(slist) ) {
     printf("\nWarning: Liste nicht leer (destroy_slist)\n");
     printf("\n         Liste nicht geloescht !!!\n");
     return slist;
  }
  if (slist->start) free(slist->start);
  if (slist) free(slist);

  return NULL;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* Listengroesse
                                  */
/* -------------------------------------------------------------- */
long slist_size(sList *slist)
{
  return  slist->size;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* Leere Liste
                                  */
/* -------------------------------------------------------------- */
int slist_empty(sList *slist)
{
  return  !slist->size;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* Liste leeren
                                  */
/* -------------------------------------------------------------- */
void delete_slist(sList *slist, void (*_delete_function)(void *))
{
  sList_Item *item,*temp;

  item = slist->start;

  while ( item != slist->end )
  {
    if (_delete_function) _delete_function(item->member);
    temp = item;
    item = item->next;
    temp=destroy_slist_item(slist,temp);
  }
  slist->size=0;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* member in der Liste einfuegen (direction=-1 --> vor  item)
                 (direction= 1 --> nach item)
                                  */
/* -------------------------------------------------------------- */
sList_Item *put_slist_member(sList *slist, sList_Item *item, int direction,
                  void *member)
{
/*  List_Item *temp; */
  sList_Item *new_item;

  if ( item == NULL ) return NULL;

  new_item=create_slist_item();
  new_item->member=member;

  if ( direction == -1 ) {
    if(item == slist->end) slist->end=new_item;
    else                   *new_item=*item;
    new_item->next=new_item;
    slist->size++;
    return new_item;
  }
  else if ( direction == 1 ) {
    if(item == slist->start) slist->start=new_item;
    else                   *new_item=*item;
    new_item->next=new_item;
    slist->size++;
    return new_item;
  }
  return NULL;
}
/* -------------------------------------------------------------- */




/* =========  List_Item : Konstruktor und Destruktor  ========= */

/* -------------------------------------------------------------- */
/* item erzeugen
                                                                  */
/* -------------------------------------------------------------- */
List_Item *create_list_item(void)
{
  List_Item *item;

  item = (List_Item *) malloc(sizeof(List_Item));

  if ( item == NULL ) {
     printf("\nNicht genug Speicher um einen neuen Listen-Item zu erstellen\n");
  }

  return item;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* item loeschen
                                                                  */
/* -------------------------------------------------------------- */
List_Item *destroy_list_item(LList *list, List_Item *item)
{
  if ( item == NULL ) {
     printf("\nItem ist ein Nullzeiger (destroy_list_item)\n");
  }
  else if ( item == list->start ) {
     printf("\nSentinel kann nicht geloescht werden (destroy_list_item) !!!\n");
  }
  else {
    item->last->next = item->next;
    item->next->last = item->last;
    free(item);
    item=NULL;
  }

  return NULL;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* current-item loeschen
                                                                  */
/* -------------------------------------------------------------- */
List_Item *destroy_list_item_current(LList *list)
{
  return destroy_list_item(list,list->current);
}
/* -------------------------------------------------------------- */




/* =========  List : Konstruktor, Destruktor usw. ========= */

/* -------------------------------------------------------------- */
/* Liste: Konstruktor
                                                                  */
/* -------------------------------------------------------------- */
LList  *create_list(void)
{
  LList *list;

  list = (LList *) malloc(sizeof(LList));

  if ( list == NULL ) {
     printf("\nNicht genug Speicher um eine neue Liste zu erstellen\n");
     return NULL;
  }
  list->size=0;
  list->start = create_list_item();
  list->start->next = list->start->last = list->start;
  list->current = list->iterator = list->start;

  return list;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* Liste: Destruktor
                                                                  */
/* -------------------------------------------------------------- */
LList  *destroy_list(LList *list)
{
  if ( !list_empty(list) ) {
     printf("\nWarning: Liste nicht leer (destroy_list)\n");
     printf("\n         Liste nicht geloescht !!!\n");
     return list;
  }
  if (list->start) free(list->start);
  if (list) free(list);

  return NULL;
}
/* -------------------------------------------------------------- */



/* -------------------------------------------------------------- */
/* Listengroesse
                                                                  */
/* -------------------------------------------------------------- */
long list_size(LList *list)
{
  return  list->size;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* Leere Liste
                                                                  */
/* -------------------------------------------------------------- */
int list_empty(LList *list)
{
  return  !list->size;
}
/* -------------------------------------------------------------- */

/* -------------------------------------------------------------- */
/* Liste leeren
                                                                  */
/* -------------------------------------------------------------- */
void delete_list(LList *list, void (*_delete_function)(void *))
{
  List_Item *item,*temp;

  item = list->start->next;

  while ( item != list->start )
  {
    if (_delete_function) _delete_function(item->member);
    temp = item;
    item = item->next;
    temp=destroy_list_item(list,temp);
  }
  list->size=0;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* Item und Member loeschen und aus der Liste entfernen
   Funktion noch nicht fertig !!
   geaendert am 17.11.98
                                                                  */
/* -------------------------------------------------------------- */
void destroy_list_item_member(LList *list, List_Item *item,
                 void (*_delete_function)(void *))
{
  List_Item *temp;

  temp = list->start->next;

  while ( temp != list->start && temp!=NULL )
  {
    if (temp == item) {
      if (_delete_function) _delete_function(temp->member);
      temp=destroy_list_item(list,temp);
      list->size--;
      break;
    }
    else temp=temp->next;
  }

  return;
}
/* -------------------------------------------------------------- */



/* -------------------------------------------------------------- */
/* Liefert niedigsten freien Feldindex
                                                                  */
/* -------------------------------------------------------------- */
long get_lowest_free (LList *list)
{
  List_Item *temp;
  long pos=-1;

  temp=list->start->next;

  while ( temp != list->start )
  {
    pos++;
    if ( temp->member == NULL ) return pos;
    temp=temp->next;
  }

  return -1;
}
/* -------------------------------------------------------------- */



/* =========  Member-Funktionen  ========= */

/* -------------------------------------------------------------- */
/* Item und Member loeschen und aus der Liste entfernen
                                                                  */
/* -------------------------------------------------------------- */
void destroy_list_member_pos(LList *list, long pos,
                 void (*_delete_function)(void *))
{
  List_Item *temp;

  temp = list->start->next;

  if ( temp != list->start && temp!=NULL )
  {
    temp->member=get_list_member_pos(list,pos);
    if (temp->member) {
      if (_delete_function) _delete_function(temp->member);
    }
    temp=destroy_list_item(list,temp);
    list->size--;
  }

  return;
}
/* -------------------------------------------------------------- */



/* -------------------------------------------------------------- */
/* member in der Liste einfuegen (direction=-1 --> vor  item)
                                 (direction= 1 --> nach item)
                                                                  */
/* -------------------------------------------------------------- */
List_Item *put_list_member(LList *list, List_Item *item, int direction,
                              void *member)
{
/*  List_Item *temp; */
  List_Item *new_item;

  if ( item == NULL ) return NULL;

  new_item=create_list_item();
  new_item->member=member;

  if ( direction == -1 ) {
    new_item->last=item->last;
    new_item->next=item;
    new_item->last->next=new_item;
    item->last=new_item;
    list->size++;
    return new_item;
  }
  else if ( direction == 1 ) {
    new_item->last=item;
    new_item->next=item->next;
    item->next->last=new_item;
    item->next=new_item;
    list->size++;
    return new_item;
  }
  return NULL;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* item aus der Liste holen (Suchen)
                                                                  */
/* -------------------------------------------------------------- */
List_Item *get_list_member(LList *list, void *member)
{
  List_Item *temp;

  temp=list->start->next;
  while ( temp != list->start )
  {
    if ( temp->member == member ) return temp;
    temp=temp->next;
  }
  return NULL;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* Member in der Position pos aus der Liste holen
   ( pos von 0 bis size-1 ) (Suchen)
                                                                  */
/* -------------------------------------------------------------- */
void *get_list_member_pos(LList *list, long pos)
{
  return get_list_item_content(list,get_list_item_pos(list,pos));
}
/* -------------------------------------------------------------- */



/* -------------------------------------------------------------- */
/* member am Ende der Liste einfuegen
                                                                  */
/* -------------------------------------------------------------- */
List_Item *append_list_member(LList *list, void *member)
{
  List_Item *temp;
  int direction=1;

  temp=put_list_member(list,list->start->last,direction,member);

  return temp;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* member am Ende der Liste einfuegen
   Returnwert ist die Position in der Liste (Hier: Size-1)
                                                                  */
/* -------------------------------------------------------------- */
long append_listV0(LList *list, void *member)
{
  List_Item *temp;
  long pos=-1;

  temp=append_list_member(list,member);
  if (temp) pos=list_size(list)-1;
  return pos;
}
/* -------------------------------------------------------------- */

/* -------------------------------------------------------------- */
/* member am Ende der Liste einfuegen
   Returnwert ist die Laenge der Liste (Hier: size)
                                                                  */
/* -------------------------------------------------------------- */
long append_list(LList *list, void *member)
{
  List_Item *temp;
  long pos=-1;

  temp=append_list_member(list,member);
  if(temp) pos=list_size(list); /* OK 17.12.1999 */
  return pos;
}
/* -------------------------------------------------------------- */




/* -------------------------------------------------------------- */
/* member am Anfang der Liste einfuegen
                                                                  */
/* -------------------------------------------------------------- */
List_Item *insert_list_member(LList *list, void *member)
{
  List_Item *temp;
  int direction=1;

  temp=put_list_member(list,list->start,direction,member);

  return temp;
}
/* -------------------------------------------------------------- */




/* -------------------------------------------------------------- */
/* new_member hinter member in der Liste einfuegen
                                                                  */
/* -------------------------------------------------------------- */
void insert_list_after(LList *list, void *member, void *new_member)
{
  int direction=1;

  put_list_member(list,get_list_member(list,member),
                       direction, new_member);

  return;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* new_member vor member in der Liste einfuegen
                                                                  */
/* -------------------------------------------------------------- */
void insert_list_bevor(LList *list, void *member, void *new_member)
{
  put_list_member(list,get_list_member(list,member),
                       -1, new_member);

  return;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* member an der Position pos in der Liste einfuegen
                                                                  */
/* -------------------------------------------------------------- */
void insert_list_pos(LList *list, long pos, void *member)
{
  int direction=1;

/*  if ( pos<0 || pos > list_size(list)-1 ) return;
  else */
  if ( pos==0 )
    put_list_member(list,list->start,direction,member);
  else
    put_list_member(list,get_list_item_pos(list,pos-1),direction,member);

  return;
}
/* -------------------------------------------------------------- */



/* -------------------------------------------------------------- */
/* member an der Position pos in der Liste eintragen
                                                                  */
/* -------------------------------------------------------------- */
void put_list_pos(LList *list, long pos, void *member)
{
  List_Item *temp;
  long i=0;

  temp = list->start->next;

  while ( temp != list->start )
  {
    if ( i == pos ) break;
    temp=temp->next;
    i++;
  }

  if ( temp != list->start && i == pos ) {
    temp->member=member;
  }

  return;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* member aus der Liste entfernen
                                                                  */
/* -------------------------------------------------------------- */
void delete_list_member(LList *list, void *member)
{
  List_Item *temp;

  if (  (temp=get_list_member(list,member)) != NULL )
  {
    list->current=temp->next;
    temp=destroy_list_item(list,temp);
    list->size--;
  }
  else
  {
    printf("\nListen-Item nicht gefunden (delete_list_member) !!!\n");
  }

  return;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* member aus der Liste entfernen mit Destruktor
                                                                  */
/* -------------------------------------------------------------- */
long remove_list_member(LList *list, void *member,void (*_delete_function)(void *))
{
  List_Item *temp;

  if (  (temp=get_list_member(list,member)) != NULL )
  {
    list->current=temp->last;
    if (_delete_function) _delete_function(temp->member);
    temp=destroy_list_item(list,temp);
    list->size--;
  }
  else
  {
    printf("\nListen-Item nicht gefunden (delete_list_member) !!!\n");
  }

  return list->size;
}
/* -------------------------------------------------------------- */



/* -------------------------------------------------------------- */
/* member suchen mit Vergleichsfunktion  (Suchen)
                                                                  */
/* -------------------------------------------------------------- */
List_Item *search_list_member(LList *list, void *member,
                                int (*_compare_function)(void *, void *))
{
  List_Item *item;

  item = list->start->next;

  while ( item != list->start )
  {
    if ( _compare_function(item->member,member) == 0 ) return item;
    item = item->next;
  }
  return NULL;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* Element suchen mit Vergleichsfunktion  (Suchen)
   Hier mit Vorsicht benutzen (kein Destruktor)
                                                                  */
/* -------------------------------------------------------------- */
void *search_list(LList *list, void *member,
                int (*_compare_function)(void *, void *))
{
  return get_list_item_content(list,
      search_list_member(list,member,_compare_function));
}
/* -------------------------------------------------------------- */


/* =========  Item-Funktionen  ========= */

/* -------------------------------------------------------------- */
/* Item-Inhalt aus der Liste holen
                                                                  */
/* -------------------------------------------------------------- */
void *get_list_item_content(LList *list, List_Item *item)
{
  if ( item == NULL || item == list->start ) return NULL;
  else return item->member;
}
/* -------------------------------------------------------------- */



/* -------------------------------------------------------------- */
/* Item-Position aus der Liste holen ( von 0 bis size-1 ) (Suchen)
                                                                  */
/* -------------------------------------------------------------- */
long get_list_item_pos_list(LList *list, List_Item *item)
{
  List_Item *temp;
  long pos=0;

  temp = list->start->next;

  while ( temp != list->start )
  {
    if ( temp == item ) break;
    temp=temp->next;
    pos++;
  }

  if ( temp != list->start && temp == item ) return pos;
  else return -1;
}
/* -------------------------------------------------------------- */

/* -------------------------------------------------------------- */
/* Item in der Position pos aus der Liste holen
   ( pos von 0 bis size-1 ) (Suchen) Version 0
                                                                  */
/* -------------------------------------------------------------- */
List_Item *get_list_item_pos_V0(LList *list, long pos)
{
  List_Item *temp;
  long i=0;

  temp = list->start->next;

  while ( temp != list->start )
  {
    if ( i == pos ) break;
    temp=temp->next;
    i++;
  }

  if ( temp != list->start && i == pos ) return temp;
  else return NULL;
}
/* -------------------------------------------------------------- */

/* -------------------------------------------------------------- */
/* Item in der Position pos aus der Liste holen
   ( pos von 0 bis size-1 ) (Suchen)
                                                                  */
/* -------------------------------------------------------------- */
List_Item *get_list_item_pos_V1(LList *list, long pos)
{
  List_Item *temp;
  long i=0;

  temp = list->start->next;
  for (i=0; i<pos; i++) temp=temp->next;
  if ( temp != list->start && i==pos ) return temp;
  else return NULL;
}
/* -------------------------------------------------------------- */



/* -------------------------------------------------------------- */
/* Item in der Position pos aus der Liste holen
   ( pos von 0 bis size-1 ) (Suchen)
                                                                  */
/* -------------------------------------------------------------- */
List_Item *get_list_item_pos(LList *list, long pos)
{
  List_Item *temp;
  long i=0;
  long half=(list->size) >> 1;

  if (pos<=half) {
    temp = list->start->next;
    for (i=0; i<pos; i++) temp=temp->next;
  }
  else {
    temp = list->start->last;
    for (i=list->size-1; i>pos; i--) temp=temp->last;
  }


  if ( temp != list->start && i==pos ) return temp;
  else return NULL;
}
/* -------------------------------------------------------------- */




/* -------------------------------------------------------------- */
/* Erstes Item der Liste holen
                                                                  */
/* -------------------------------------------------------------- */
List_Item *list_item_first(LList *list)
{
  List_Item *temp;

  temp = list->start->next;

  if ( temp != list->start )
  {
    return temp;
  }
  else return NULL;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* Letztes Item der Liste holen
                                                                  */
/* -------------------------------------------------------------- */
List_Item *list_item_last(LList *list)
{
  List_Item *temp;

  temp = list->start->last;

  if ( temp != list->start )
  {
    return temp;
  }
  else return NULL;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* Item in der Position pos loeschen ( pos von 0 bis size-1 )
                                                                  */
/* -------------------------------------------------------------- */
void delete_list_item_pos(LList *list, long pos)
{
  List_Item *temp;
  long i=0;

  temp = list->start->next;

  while ( temp != list->start )
  {
    if ( i == pos ) break;
    i++;
    temp=temp->next;
  }
  if ( temp != list->start && i == pos ) {
    free(temp->member);
    temp->member=NULL;
  }
  return;
}
/* -------------------------------------------------------------- */

/* -------------------------------------------------------------- */
/* Item in der Position pos loeschen ( pos von 0 bis size-1 )
                                                                  */
/* -------------------------------------------------------------- */
void destroy_list_item_pos(LList *list, long pos)
{
  List_Item *temp;
  long i=0;

  temp = list->start->next;

  while ( temp != list->start )
  {
    if ( i == pos ) break;
    i++;
    temp=temp->next;
  }
  if ( temp != list->start && i == pos ) {
    temp=destroy_list_item(list,temp);
    list->size--;
  }
  return;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* Item in der Position pos loeschen ( pos von 0 bis size-1 )
                                                                  */
/* -------------------------------------------------------------- */
void delete_list_item_last(LList *list)
{
  List_Item *temp;

  temp=list_item_last(list);

  if ( temp )
  {
    free(temp->member);
    temp=destroy_list_item(list,temp);
    list->size--;
  }

  return;
}
/* -------------------------------------------------------------- */


/* =========  Iterator-Funktionen  ========= */


/* -------------------------------------------------------------- */
/* Iterator initialisierren
                                                                  */
/* -------------------------------------------------------------- */
void list_iterator_init(LList *list)
{
  list->iterator=list->start;
  return;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* aktueller Iterator
                                                                  */
/* -------------------------------------------------------------- */
List_Item *list_iterator_current(LList *list)
{
  if (list->iterator == list->start) return NULL;
  else return list->iterator;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* Iterator nach vorne bewegen
                                                                  */
/* -------------------------------------------------------------- */
List_Item *list_iterator_next(LList *list)
{
  list->iterator=list->iterator->next;
  return list_iterator_current(list);
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* Iterator zurueck bewegen
                                                                  */
/* -------------------------------------------------------------- */
List_Item *list_iterator_preview(LList *list)
{
  list->iterator=list->iterator->last;
  return list_iterator_current(list);
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* Iterator am Anfang positionieren
                                                                  */
/* -------------------------------------------------------------- */
List_Item *list_iterator_first(LList *list)
{
  list->iterator=list->start->next;
  return list_iterator_current(list);
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* Iterator am Ende positionieren
                                                                  */
/* -------------------------------------------------------------- */
List_Item *list_iterator_last(LList *list)
{
  list->iterator=list->start->last;
  return list_iterator_current(list);
}
/* -------------------------------------------------------------- */


/* =========  Current-Funktionen  ========= */

/* -------------------------------------------------------------- */
/* current initialisieren
                                                                  */
/* -------------------------------------------------------------- */
void list_current_init(LList *list)
{
  list->current=list->start;
  return;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* current auf naechstes item bewegen
                                                                  */
/* -------------------------------------------------------------- */
void *list_current_next(LList *list)
{
  list->current=list->current->next;
  if (list->current == list->start) return NULL;
  else return list->current;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* current auf vorheriges item bewegen
                                                                  */
/* -------------------------------------------------------------- */
void *list_current_preview(LList *list)
{
  list->current=list->current->last;
  if (list->current == list->start) return NULL;
  else return list->current;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* Aktuelles member in der Liste holen
                                                                  */
/* -------------------------------------------------------------- */
void *get_list_current(LList *list)
{

  return get_list_item_content(list,list->current);
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* Naechstes member in der Liste holen
                                                                  */
/* -------------------------------------------------------------- */
void *get_list_next(LList *list)
{
  if ( list_current_next(list) != NULL )
    return get_list_item_content(list,list->current);
  else
    return NULL;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* Vorheriges member in der Liste holen
                                                                  */
/* -------------------------------------------------------------- */
void *get_list_preview(LList *list)
{
  if ( list_current_preview(list) != NULL )
    return get_list_item_content(list,list->current);
  else
    return NULL;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* Kopfelement der Liste holen
                                                                  */
/* -------------------------------------------------------------- */
void *get_list_head(LList *list)
{
  List_Item *temp;
  void *member;

  temp=list->start->next;
  list->current=list->start->next;

  if ( temp != list->start ) {
    member=temp->member;
    temp=destroy_list_item(list,temp);
    list->size--;
    return member;
  }
  else
    return NULL;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* Schwanzelement der Liste holen
                                                                  */
/* -------------------------------------------------------------- */
void *get_list_tail(LList *list)
{
  List_Item *temp;
  void *member;

  temp=list->start->last;
  list->current=list->start->next;

  if ( temp != list->start ) {
    member=temp->member;
    temp=destroy_list_item(list,temp);
    list->size--;
    return member;
  }

  else
    return NULL;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* Liste ausgeben
                                                                  */
/* -------------------------------------------------------------- */
void print_list(LList *list, void (*_print_function)(void *))
{
  List_Item *item;

  item = list->start->next;

  while ( item != list->start )
  {
    _print_function(item->member);
    item = item->next;
  }
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/*
                                                                  */
/* -------------------------------------------------------------- */
void write_list(LList *list, FILE *fp, void (*_write_function)(FILE *fp, void *))
{
  List_Item *item;

  item = list->start->next;

  while ( item != list->start )
  {
    _write_function(fp,item->member);
    item = item->next;
  }
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/*
                                                                  */
/* -------------------------------------------------------------- */
void load_list(LList *list, FILE *fp, void (*_load_function)(FILE *fp, void *))
{
  List_Item *item;

  item = list->start->next;

  while ( item != list->start )
  {
    _load_function(fp,item->member);
    item = item->next;
  }
}
/* -------------------------------------------------------------- */





/* =========  Stack-Funktionen  ========= */

/* -------------------------------------------------------------- */
/* Stack erzeugen
                                                                  */
/* -------------------------------------------------------------- */
Stack  *create_stack(void)
{
  Stack *stack;

  stack = (Stack *) malloc(sizeof(Stack));

  if ( stack == NULL ) {
     printf("\nNicht genug Speicher um einen neuen Stack zu erstellen\n");
     return NULL;
  }

  stack->list=create_list();

  if (stack->list) return stack;
  else return NULL;
}
/* -------------------------------------------------------------- */



/* -------------------------------------------------------------- */
/* Stackgroesse
                                                                  */
/* -------------------------------------------------------------- */
long stack_size(Stack *stack)
{
  return  list_size(stack->list);
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* Leerer Stack
                                                                  */
/* -------------------------------------------------------------- */
int stack_empty(Stack *stack)
{
  return  list_empty(stack->list);
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* member aus dem Stack holen
                                                                  */
/* -------------------------------------------------------------- */
void *top_stack_member(Stack *stack)
{
  return get_list_item_content(stack->list,
      list_item_first(stack->list));
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* member in Stack einfuegen
                                                                  */
/* -------------------------------------------------------------- */
void push_stack_member(Stack *stack, void *member)
{
  insert_list_member(stack->list,member);

  return;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* member aus dem Stack entfernen und holen
                                                                  */
/* -------------------------------------------------------------- */
void *pop_stack_member(Stack *stack)
{
  return get_list_head(stack->list);
}
/* -------------------------------------------------------------- */



/* =========  Queue-Funktionen  ========= */

/* -------------------------------------------------------------- */
/* Queue erzeugen
                                                                  */
/* -------------------------------------------------------------- */
Queue  *create_queue(void)
{
  Queue *queue;

  queue = (Queue *) malloc(sizeof(Queue));

  if ( queue == NULL ) {
     printf("\nNicht genug Speicher um eine neue Queue zu erstellen\n");
     return NULL;
  }

  queue->list=create_list();

  if (queue->list) return queue;
  else return NULL;
}
/* -------------------------------------------------------------- */



/* -------------------------------------------------------------- */
/* Queuegroesse
                                                                  */
/* -------------------------------------------------------------- */
long queue_size(Queue *queue)
{
  return  list_size(queue->list);
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* Leere Queue
                                                                  */
/* -------------------------------------------------------------- */
int queue_empty(Queue *queue)
{
  return  list_empty(queue->list);
}
/* -------------------------------------------------------------- */

/* -------------------------------------------------------------- */
/* member aus der Queue holen
                                                                  */
/* -------------------------------------------------------------- */
void *front_queue_member(Queue *queue)
{
  return get_list_item_content(queue->list,
      list_item_last(queue->list));
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* member in Queue einfuegen
                                                                  */
/* -------------------------------------------------------------- */
void put_queue_member(Queue *queue, void *member)
{
  insert_list_member(queue->list,member);

  return;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* member aus der Queue entfernen und holen
                                                                  */
/* -------------------------------------------------------------- */
void *get_queue_member(Queue *queue)
{
  return get_list_tail(queue->list);
}
/* -------------------------------------------------------------- */





/* -------------------------------------------------------------- */
/* Ab hier noch nicht benutzen !!! */
/* Noch nicht getestet (noch nicht fertig) */
/* -------------------------------------------------------------- */

List_Item *merge_sort_list(LList *list, int (*_compare_function)(void *, void *));
List_Item *merge_sort_algo_list(List_Item *rest, List_Item *item_NULL,
                                                long lenght, int (*_compare_function)(void *, void *));
List_Item *merge_list(List_Item *item1, List_Item *item2, List_Item *item_NULL,
                                          int (*_compare_function)(void *, void *));



/* -------------------------------------------------------------- */
/* Liste sortieren
                                                                  */
/* -------------------------------------------------------------- */
List_Item *merge_sort_list(LList *list, int (*_compare_function)(void *, void *))
{

  return merge_sort_algo_list(list->start->next,list->start,
                                  list_size(list),_compare_function);

}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* merge sort algorithm ( O(nlog(n)) )
                                                                  */
/* -------------------------------------------------------------- */
List_Item *merge_sort_algo_list(List_Item *rest, List_Item *item_NULL,
                                                                long lenght, int (*_compare_function)(void *, void *))
{
  List_Item *temp;

  if ( rest == item_NULL ) return NULL;
  else if ( lenght == 1 ) {
          temp=rest;
          rest=rest->next;
          return temp;
  }
  else return merge_list(
       merge_sort_algo_list(rest,item_NULL,lenght/2,_compare_function),
       merge_sort_algo_list(rest,item_NULL,(lenght+1)/2,_compare_function),
           item_NULL,_compare_function
       );
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* Listen zusammenfuegen
                                                                                                  */
/* -------------------------------------------------------------- */
List_Item *merge_list(List_Item *item1, List_Item *item2, List_Item *item_NULL,
                                          int (*_compare_function)(void *, void *))
{
  List_Item *first=NULL, *last=NULL, *temp;

  for(;;) {
/*  while (1) {*/
     if ( item1 == item_NULL ) {
                 if ( first == NULL ) first=item2;
                 else last->next=item2;
                 break;
         }
     else if ( item2 == NULL ) {
                 if ( first == NULL ) first=item1;
                 else last->next=item1;
                 break;
         }
     else if ( _compare_function(item2,item1) ) {
             temp=item1;
                 item1=item1->next;
     }
     else {
             temp=item2;
                 item2=item2->next;
     }
     temp->next=item_NULL;
         if ( first == NULL ) first=temp;
         else {
                 last->next=temp;
                 last=temp;
     }
 }
 return first;
}
/* -------------------------------------------------------------- */


/* =========  BinTree_Item : Konstruktor und Destruktor  ========= */

/* -------------------------------------------------------------- */
/* BinTree_Item erzeugen
                                                                  */
/* -------------------------------------------------------------- */
BinTree_Item *create_bintree_item(void)
{
  BinTree_Item *item;

  item = (BinTree_Item *) malloc(sizeof(BinTree_Item));

  if ( item == NULL ) {
     printf("\nNicht genug Speicher um einen neuen BinTree-Item zu erzeugen\n");
     return NULL;
  }
  item->left = item->right = NULL;

  return item;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* BinTree_Item zerstoeren (noch nicht fertig !!!)
   bessere Loesung ueber Zeiger auf Zeiger
                                                                  */
/* -------------------------------------------------------------- */
BinTree_Item *destroy_bintree_item(BinTree *bintree, BinTree_Item *item)
{
  if ( item == NULL ) {
     printf("\nItem ist ein Nullzeiger (destroy_bintree_item)\n");
  }
  bintree->size--;
  return NULL;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* BinTree_Item loeschen (iterativ)
                                                                  */
/* -------------------------------------------------------------- */
void delete_bintree_item(BinTree_Item **item)
{
  BinTree_Item *p,*q,**temp;

  temp=item;
  if ( *item != NULL ) {
      p=*item;
          if (p->right == NULL) {
                  *item=p->left;
                  free(p);
      }
          else if (p->left == NULL) {
                  *item=p->right;
                  free(p);
      }
          else {
                  temp=&p->left;
          while ( (*temp)->right != NULL ) temp=&(*temp)->right;
                  q=*temp, *temp=q->left;
                  /*copy_bintree_item_content(q,p);*/
                  free(q);
          }
  }
}



/* -------------------------------------------------------------- */
/* BinTree_Item loeschen (iterativ)
                                                                  */
/* -------------------------------------------------------------- */
void destroy_bintree_item_id(BinTree_Item **item,
                    void (*_delete_function)(void *))
{
  BinTree_Item *p,*q,**temp;

  temp=item;
  if ( *item != NULL ) {
      p=*item;
          if (p->right == NULL) {
                  *item=p->left;
                  free(p);
      }
          else if (p->left == NULL) {
                  *item=p->right;
                  free(p);
      }
          else {
                  temp=&p->left;
          while ( (*temp)->right != NULL ) temp=&(*temp)->right;
                  q=*temp, *temp=q->left;
                  /*copy_bintree_item_content(q,p);*/
                  if (_delete_function) _delete_function(q->member);
                  free(q);
          }
  }
}



/* =========  BinTree : Konstruktor, Destruktor usw. ========= */

/* -------------------------------------------------------------- */
/* BinTree erzeugen
                                                                  */
/* -------------------------------------------------------------- */
BinTree  *create_bintree(void)
{
  BinTree *bintree;

  bintree = (BinTree *) malloc(sizeof(BinTree));

  if ( bintree == NULL ) {
     printf("\nNicht genug Speicher um einen neuen binaeren Baum zu erzeugen\n");
     return NULL;
  }

  bintree->root = NULL;
  bintree->size = 0;

  return bintree;
}
/* -------------------------------------------------------------- */



/* -------------------------------------------------------------- */
/* BinTree zerstoeren
                                                                  */
/* -------------------------------------------------------------- */
BinTree  *destroy_bintree(BinTree *bintree)
{

  bintree = (BinTree *) malloc(sizeof(BinTree));

  if ( bintree == NULL ) {
     printf("\nBinTree existiert nicht !!! (destroy_bintree)\n");
     return NULL;
  }

  if (bintree->root){
          free(bintree->root);
          bintree->root= NULL;
  }

  free(bintree);
  bintree=NULL;

  return bintree;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* BinTree-Groesse
                                                                  */
/* -------------------------------------------------------------- */
long bintree_size(BinTree *bintree)
{
  return  bintree->size;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* Leere BinTree
                                                                  */
/* -------------------------------------------------------------- */
int bintree_empty(BinTree *bintree)
{
  return  !bintree->size;
}
/* -------------------------------------------------------------- */



/* -------------------------------------------------------------- */
/* member in BinTree einfuegen
                                                                  */
/* -------------------------------------------------------------- */
void append_bintree_member_V0(BinTree *bintree, void *member,
                           int (*_compare_function)(void *, void *))
{
  BinTree_Item *item = create_bintree_item();
  item->member=member;
  bintree->size++;
  bintree->root=append_bintree_item_V0(bintree->root,item,_compare_function);

  return;
}
/* -------------------------------------------------------------- */



/* -------------------------------------------------------------- */
/* member in BinTree einfuegen mit Identifikator-Funktion
                                                                  */
/* -------------------------------------------------------------- */
void append_bintree_member(BinTree *bintree, void *member,
                           long (*_id_function)(void *))
{
  BinTree_Item *item = create_bintree_item();
  item->member=member;
  bintree->size++;
  bintree->root=append_bintree_item(bintree->root,item,_id_function);

  return;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* member aus dem Baum entfernen (rekursiv)
                                                                  */
/* -------------------------------------------------------------- */
void delete_bintree_member_r(BinTree *bintree, void *member)
{
  BinTree_Item *temp;

  if (  (temp=get_bintree_member(bintree,bintree->root,member)) != NULL )
  {
    temp=destroy_bintree_item(bintree,temp);
    if (bintree->size) bintree->size--;
  }
  else
  {
    printf("\nBinTree-Item nicht gefunden (delete_tree_member) !!!\n");
  }

  return;
}
/* -------------------------------------------------------------- */


/* -------------------------------------------------------------- */
/* member aus dem Baum entfernen (iterativ)
                                                                  */
/* -------------------------------------------------------------- */
void delete_bintree_member(BinTree *bintree, void *member,
                            int (*_compare_function)(void *, void *))
{
  BinTree_Item **temp;

  if (  *(temp=search_bintree_item(&bintree->root,member,_compare_function)) != NULL )
  {
    *temp=destroy_bintree_item(bintree,*temp);
    if (bintree->size) bintree->size--;
  }
  else
  {
    printf("\nBinTree-Item nicht gefunden (delete_tree_member) !!!\n");
  }

  return;
}
/* -------------------------------------------------------------- */



/* -------------------------------------------------------------- */
/* BinTree_Item im Baum suchen (Suchen)
                                                                  */
/* -------------------------------------------------------------- */
BinTree_Item *get_bintree_member(BinTree *bintree, BinTree_Item *item,
                                                                 void *member)
{
  int indicator;

  if (item == NULL) return NULL;
  indicator = item->member == member;
  return indicator < 0 ?
         get_bintree_member(bintree,item->left,member) :
         indicator > 0 ?
         get_bintree_member(bintree,item->right,member) :
         item;
}
/* -------------------------------------------------------------- */



/* -------------------------------------------------------------- */
/* BinTree_Item im Baum einfuegen
                                                                  */
/* -------------------------------------------------------------- */
BinTree_Item *append_bintree_item_V0(BinTree_Item *item1, BinTree_Item *item2,
                               int (*_compare_function)(void *, void *))
{

  if (item1 == NULL)
   {
     item1=item2;
   }
  else
   {
     if ( _compare_function(item1->member,item2->member) >= 0 )
       item1->left = append_bintree_item_V0(item1->left,item2,_compare_function);
     else
       item1->right = append_bintree_item_V0(item1->right,item2,_compare_function);
   }
  return item1;
}


/* -------------------------------------------------------------- */
/* BinTree_Item im Baum einfuegen mit Identifikator-Funktion
                                                                  */
/* -------------------------------------------------------------- */
BinTree_Item *append_bintree_item(BinTree_Item *item1, BinTree_Item *item2,
                               long (*_id_function)(void *))
{

  if (item1 == NULL)
   {
     item1=item2;
   }
  else
   {
     if ( _id_function(item1->member) >= _id_function(item2->member) )
       item1->left = append_bintree_item(item1->left,item2,_id_function);
     else
       item1->right = append_bintree_item(item1->right,item2,_id_function);
   }
  return item1;
}



/* -------------------------------------------------------------- */
/* BinTree durchsuchen
                                                                  */
/* -------------------------------------------------------------- */
BinTree_Item* search_bintree(BinTree *bintree, void *member,
                    int (*_compare_function)(void *, void *))
{
  return search_bintree_item_r(bintree->root,member,_compare_function);
}


/* -------------------------------------------------------------- */
/* BinTree_Item suchen (rekursiv)
                                                                  */
/* -------------------------------------------------------------- */
BinTree_Item *search_bintree_item_r(BinTree_Item *item, void *member,
                    int (*_compare_function)(void *, void *))
{
  int indicator;

  if (item == NULL) return NULL;
  indicator = _compare_function(member,item->member);
  return indicator < 0 ?
         search_bintree_item_r(item->left,member,_compare_function) :
         indicator > 0 ?
         search_bintree_item_r(item->right,member,_compare_function) :
         item;
}


/* -------------------------------------------------------------- */
/* BinTree_Item suchen (iterativ)
                                                                  */
/* -------------------------------------------------------------- */
BinTree_Item **search_bintree_item(BinTree_Item **item, void *member,
                    int (*_compare_function)(void *, void *))
{
  BinTree_Item **temp;
  int indicator;

  temp=item;

  while (*temp != NULL && (indicator=_compare_function(member,(*temp)->member)) != 0) /* !! */
          temp= (indicator < 0 ? &(*temp)->left : &(*temp)->right);
  return temp;
}


/* -------------------------------------------------------------- */
/* Item-Inhalt aus der Baumstruktur holen
                                                                  */
/* -------------------------------------------------------------- */
void *get_bintree_item_content(BinTree *bintree, BinTree_Item *item)
{
  if ( item == NULL || bintree_empty(bintree) ) return NULL;
  else return item->member;
}
/* -------------------------------------------------------------- */



/* -------------------------------------------------------------- */
/* BinTree_Item suchen (iterativ) mit Identifikator-Funktion
                                                                  */
/* -------------------------------------------------------------- */
BinTree_Item **search_bintree_item_id(BinTree_Item **item, long id,
                    long (*_id_function)(void *))
{
  BinTree_Item **temp;
  long id_member;

  temp=item;

  while (*temp != NULL && (id_member=_id_function((*temp)->member)) != id)
          temp= (id < id_member ? &(*temp)->left : &(*temp)->right);
  return temp;
}


/* -------------------------------------------------------------- */
/* member suchen (iterativ) mit Identifikator-Funktion
                                                                  */
/* -------------------------------------------------------------- */
void *search_bintree_member_id(BinTree *bintree, long id,
                    long (*_id_function)(void *))
{
  return get_bintree_item_content(bintree,
           *search_bintree_item_id(&bintree->root,id,_id_function));
}


/* -------------------------------------------------------------- */
/* member loeschen mit Identifikator-Funktion
                                                                  */
/* -------------------------------------------------------------- */
void *delete_bintree_member_id(BinTree *bintree, long id,
                    long (*_id_function)(void *),
                    void (*_delete_function)(void *))
{
  destroy_bintree_item_id(
              search_bintree_item_id(&bintree->root,id,_id_function),
              _delete_function);
  /*destroy_bintree_item(bintree,temp);*/
  if (bintree->size) bintree->size--;

  return NULL;
}


/* -------------------------------------------------------------- */
/* Ast loeschen mit Identifikator-Funktion
                                                                  */
/* -------------------------------------------------------------- */
void *delete_bintree_id(BinTree *bintree, long id,
                    long (*_id_function)(void *),
                    void (*_delete_function)(void *))
{
  destroy_bintree_id(
              search_bintree_item_id(&bintree->root,id,_id_function),
              _delete_function);
  return NULL;
}

/* -------------------------------------------------------------- */
/* Ast von BinTree_Item loeschen
                                                                  */
/* -------------------------------------------------------------- */
void destroy_bintree_id(BinTree_Item **item,
                    void (*_delete_function)(void *))
{
  BinTree_Item **temp;

  temp=item;
  if ( *item != NULL ) {
    destroy_bintree_id(&(*item)->left,_delete_function);
    destroy_bintree_id(&(*item)->right,_delete_function);
    if (_delete_function) _delete_function((*item)->member);
    *item=NULL;
  }
}


/* -------------------------------------------------------------- */
/* member in BinTree einfuegen (iterativ)
                                                                  */
/* -------------------------------------------------------------- */
int insert_bintree_member(BinTree *bintree, void *member,
                           long (*_id_function)(void *))
{
  BinTree_Item **temp;
  long id_member=_id_function(member);

  temp=search_bintree_item_id(&bintree->root,id_member,_id_function);

  if (*temp == NULL) {      /* Knoten nicht gefunden */
    /*if ( !(*temp = create_bintree_item()) ) return 0;*/
    if ( (*temp = create_bintree_item()) != NULL ) return 0;
    (*temp)->member=member;
    bintree->size++;
    return 1;
  }
  else {
    printf("\n Knoten doppelt vorhanden !!!");
    return 0;
  }
}
/* -------------------------------------------------------------- */



/* -------------------------------------------------------------- */
/* BinTree ausgeben
                                                                  */
/* -------------------------------------------------------------- */
void print_bintree(BinTree *bintree, void (*_print_function)(void *))
{
  print_bintree_item(bintree->root,_print_function);
}


/* -------------------------------------------------------------- */
/* BinTree_Item ausgeben
                                                                  */
/* -------------------------------------------------------------- */
void print_bintree_item(BinTree_Item *item,
                        void (*_print_function)(void *))
{
  if (item != NULL)
   {
     print_bintree_item(item->left,_print_function);
     _print_function(item->member);
     print_bintree_item(item->right,_print_function);
   }
}
