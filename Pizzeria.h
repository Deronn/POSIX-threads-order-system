#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

unsigned int sleep(unsigned int seconds);
unsigned int rngBetweenTwoValues (unsigned int* seed, int tLow, int tHigh);
void *order(void *oid);

//Είσοδος και δεδομένα
const int Ncook = 2; //παρασκευαστές
const int Noven = 15; //φούρνοι
const int Npacker = 2; //υπάλληλοι πακεταρίσματος
const int Ndeliverer = 10; //διανομείς
const int Torderlow = 1; //λεπτό
const int Torderhigh= 3; //λεπτά
const int Norderlow = 1; //πίτσες
const int Norderhigh = 5; //πίτσες
const int Pplain = 60; //πιθανότητα να είναι απλή η πίτσα
const int Tpaymentlow = 1; //λεπτό
const int Tpaymenthigh = 3; //λεπτά
const int Pfail = 10; //πιθανότητα αποτυχίας της παραγγελίας
const int Cplain = 10; //κόστος απλής
const int Cspecial = 12; //κόστος σπέσιαλ
const int Tprep = 1; //λεπτό
const int Tbake = 10; //λεπτά
const int Tpack = 1; //λεπτά
const int Tdellow = 5; //λεπτά
const int Tdelhigh=15; //λεπτά

//Παράμετροι εισόδου
int Ncust; //αριθμός πελατών
unsigned int seed; //σπόρος για τη γεννήτρια τυχαίων αριθμών

//Βοηθητικές μεταβλητές
int cooks = Ncook;
int ovens = Noven;
int deliverers = Ndeliverer;

//Ποσότητες και στατιστικά
int totalMoney=0;

int amountSpecial = 0;
int amountPlain = 0;

int amountTotalSpecial = 0;
int amountTotalPlain = 0;

int suceededtransaction = 0;
int failedtransaction = 0;

int sumDeliveryTime = 0;
int maxDeliveryTime = 0;

int sumOrderTime = 0;
int maxOrderTime = 0;

int sumColdPizzaTime = 0;
int maxColdPizzaTime = 0;


int packer=1;
int screen=1;

int changeMoney=1;

int changeSTransactions=1;
int changeFTransactions=1;

int changesumTimeWaitingOrder=1;
int changemaxTimeWaitingOrder=1;

int changesumDeliveryTime=1;
int changemaxDeliveryTime=1;

int changesumColdPizzaTime=1;
int changemaxColdPizzaTime=1;


//Αρχικοποίηση για mutexes
pthread_mutex_t orderLock;
pthread_mutex_t cookLock;
pthread_mutex_t ovenLock;
pthread_mutex_t packerLock;
pthread_mutex_t delivererLock;
pthread_mutex_t moneyLock;
pthread_mutex_t screenLock;

pthread_mutex_t sTransactionLock;
pthread_mutex_t fTransactionLock;

pthread_mutex_t sumOrderLock;
pthread_mutex_t maxOrderLock;

pthread_mutex_t sumDelLock;
pthread_mutex_t maxDelLock;

pthread_mutex_t sumColdPizzaLock;
pthread_mutex_t maxColdPizzaLock;

//Conditions για threads
pthread_cond_t orderWait;
pthread_cond_t cookWait;
pthread_cond_t ovenWait;
pthread_cond_t packerWait;
pthread_cond_t delivererWait;
pthread_cond_t moneyWait;
pthread_cond_t screenWait;

pthread_cond_t sTransactionWait;
pthread_cond_t fTransactionWait;
pthread_cond_t sDeliveryWait;

pthread_cond_t sumOrderWait;
pthread_cond_t maxOrderWait;

pthread_cond_t sumDelWait;
pthread_cond_t maxDelWait;

pthread_cond_t sumColdPizzaWait;
pthread_cond_t maxColdPizzaWait;
