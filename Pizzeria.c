#include "Pizzeria.h"

//Funtions..

//Επιστρέφει μία τυχαία τιμή ανάμεσα στον tLow, tHigh σύμφωνα με το seed.  
unsigned int rngBetweenTwoValues (unsigned int* seed, int tLow, int tHigh){
    return ((rand_r(seed) % (tHigh - tLow + 1)) + tLow);
}


//Customer order function
void *order(void *id){

	//Local variables
 	int oid= *(int*)id;
    int pitses;
    int rc;
    // Time variables
    struct timespec custStart, orderStart, orderEnd, readyPizza, packEnd, PizzaDelivered;    
	int tCustStart, tOrderStart, tOrderEnd, tReadyPizza, tPackEnd, tPizzaDelivered; 
	int tTimeWaitingOrder;
	int roadTime;
    int tDeliveryTime; 
    int tColdPizzaTime; 
    int tPackTime;


//Αρχικοποίηση της ώρας που συνδέεται ο πελάτης
    clock_gettime(CLOCK_REALTIME,&custStart);
    tCustStart =custStart.tv_sec;

    if(oid!=1){
    	sleep(rngBetweenTwoValues(&seed,Torderlow,Torderhigh)); //τυχαίο χρονικό διάστημα μετά τον προηγούμενο πελάτη(αν δεν είναι ο πρώτος πελάτης)
    }

    clock_gettime(CLOCK_REALTIME,&orderStart);
    tOrderStart = orderStart.tv_sec;             

//Start Order
    rc=pthread_mutex_lock(&orderLock); //κλειδώνει την σύνδεση
    while(cooks==0){
    	//busy cookers
    	rc = pthread_cond_wait(&orderWait, &orderLock);
    }
    clock_gettime(CLOCK_REALTIME,&orderEnd);
    tOrderEnd =orderEnd.tv_sec;
    tTimeWaitingOrder = tOrderEnd - tOrderStart; 

    cooks--;
    
//πάρε αριθμό απο πίτσες
    pitses=rngBetweenTwoValues(&seed,Norderlow,Norderhigh);
    
//βρες είδος πίτσας
	for(int i = 0; i < pitses; i++){
		if(rand_r(&seed)%100 < Pplain){
			amountPlain++;
		}else{
			amountSpecial++;
		}
	}

//αναμονή πληρωμής
    sleep(rngBetweenTwoValues(&seed,Tpaymentlow,Tpaymenthigh));

//πιθανότητα 10% αποτυχίας
    if(rand_r(&seed)%100<10){//apetyxe
    	cooks++;
   		rc = pthread_cond_signal(&orderWait);
   		rc = pthread_mutex_unlock(&orderLock);
//End Order (failed)

//Αλλαγή μετρητή αποτυχημένων συναλλαγών 
    	rc=pthread_mutex_lock(&fTransactionLock);
    	while(!changeFTransactions)
    		rc = pthread_cond_wait(&fTransactionWait, &fTransactionLock);
    	changeFTransactions=0;

    	failedtransaction++;

    	changeFTransactions=1;
    	rc = pthread_cond_signal(&fTransactionWait);
    	rc=pthread_mutex_unlock(&fTransactionLock);

//μήνυμα εξόδου αποτυχίας
    	rc=pthread_mutex_lock(&screenLock);
    	while(!screen)
    		rc = pthread_cond_wait(&screenWait, &screenLock);
    	screen=0;
    	printf("H paraggelia me arithmo %d apetyxe.\n", oid);

    	screen=1;
    	rc = pthread_cond_signal(&screenWait);
    	rc=pthread_mutex_unlock(&screenLock);
    	pthread_exit(NULL);
    }
    else{//petyxainei kata 90%

//Αλλαγή μετρητή επιτυχημένων συναλλαγών 
    	rc=pthread_mutex_lock(&sTransactionLock);
    	while(!changeFTransactions)
    		rc = pthread_cond_wait(&sTransactionWait, &sTransactionLock);
    	changeSTransactions=0;

    	suceededtransaction++;

    	changeSTransactions=1;
    	rc = pthread_cond_signal(&sTransactionWait);
    	rc=pthread_mutex_unlock(&sTransactionLock);

//μήνυμα εξόδου επιτυχίας
    	rc=pthread_mutex_lock(&screenLock);
    	while(!screen)
    		rc = pthread_cond_wait(&screenWait, &screenLock);
    	screen=0;
    	printf(" H paraggelia me arithmo %d kataxwrithike.\n", oid);
    	screen=1;
    	rc = pthread_cond_signal(&screenWait);
    	rc=pthread_mutex_unlock(&screenLock);

//Ανανέωσε έσοδα
    	rc=pthread_mutex_lock(&moneyLock);
    	while(!changeMoney)
    		rc = pthread_cond_wait(&moneyWait, &moneyLock);
    	changeMoney=0;
    	
    	amountTotalSpecial += amountSpecial;
    	amountTotalPlain += amountPlain;

    	totalMoney += Cspecial*amountSpecial + Cplain*amountPlain;

    	changeMoney=1;
    	rc = pthread_cond_signal(&moneyWait);
    	rc=pthread_mutex_unlock(&moneyLock);
    }
    cooks++;
    rc = pthread_cond_signal(&orderWait);
    rc = pthread_mutex_unlock(&orderLock);
//End Order (succeeded)

//Start cook
    rc = pthread_mutex_lock(&cookLock);
    while(cooks==0){
    	//busy cooks
    	rc = pthread_cond_wait(&cookWait, &cookLock);
    }
    cooks--;
    sleep(Tprep);

//Start oven
    rc = pthread_mutex_lock(&ovenLock);
    while(ovens<pitses){
    	//buse ovens
    	rc = pthread_cond_wait(&ovenWait, &ovenLock);
    }
    ovens-=pitses;

    cooks++;
    rc = pthread_cond_signal(&cookWait);
    rc = pthread_mutex_unlock(&cookLock);
//End cook

    sleep(Tbake);

    clock_gettime(CLOCK_REALTIME,&readyPizza);
    tReadyPizza =readyPizza.tv_sec;

//Start packing
    rc=pthread_mutex_lock(&packerLock);
    while(!packer){
    	//buse packer
    	rc = pthread_cond_wait(&packerWait, &packerLock);
    }
    packer=0;
    sleep(Tpack);//χρόνος για πακετάρισμαδ

    clock_gettime(CLOCK_REALTIME,&packEnd);
    tPackEnd =packEnd.tv_sec;

    tPackTime = tPackEnd - tCustStart; 

//Μήνυμα εξόδου έτοιμης παραγγελίας
    rc = pthread_mutex_lock(&screenLock);
    while(!screen)
    	rc = pthread_cond_wait(&screenWait, &screenLock);
    screen=0;
    printf(" H paraggelia me arithmo %d etoimastike se %d lepta.\n", oid, tPackTime);
    screen=1;
    rc = pthread_cond_signal(&screenWait);
    rc = pthread_mutex_unlock(&screenLock);

    ovens+=pitses;
    rc = pthread_cond_broadcast(&ovenWait);
	rc = pthread_mutex_unlock(&ovenLock);
//End ovens

	packer=1;
	rc = pthread_cond_signal(&packerWait);
    rc=pthread_mutex_unlock(&packerLock);
//End packing

//Start delivery
    rc = pthread_mutex_lock(&delivererLock);
    while(deliverers=0)
    	rc = pthread_cond_wait(&delivererWait, &delivererLock);
    deliverers--;

    roadTime=rngBetweenTwoValues(&seed, Tdellow, Tdelhigh);
	sleep(roadTime);//χρόνος αποστολής

    clock_gettime(CLOCK_REALTIME,&PizzaDelivered);
    tPizzaDelivered =PizzaDelivered.tv_sec;

    tDeliveryTime = tPizzaDelivered - tCustStart; //στιγμή παράδοσης

//Μήνυμα παράδοσης
    rc = pthread_mutex_lock(&screenLock);
    while(!screen)
    	rc = pthread_cond_wait(&screenWait, &screenLock);
    screen=0;
    printf(" H paraggelia me arithmo %d paradothike se %d lepta.\n", oid, tDeliveryTime);
    screen=1;
    rc = pthread_cond_signal(&screenWait);
    rc = pthread_mutex_unlock(&screenLock);

    sleep(roadTime);//χρόνος επιστροφής διανομέα

    deliverers++;
    rc = pthread_cond_signal(&delivererWait);
    rc = pthread_mutex_unlock(&delivererLock);
//End Delivery

//Update sumDeliveryTime 
    rc=pthread_mutex_lock(&sumDelLock);
    while(!changesumDeliveryTime)
    	rc = pthread_cond_wait(&sumDelWait, &sumDelLock);
    changesumDeliveryTime =0;

    sumDeliveryTime+=tDeliveryTime;

    changesumDeliveryTime=1;
    rc = pthread_cond_signal(&sumDelWait);
    rc = pthread_mutex_unlock(&sumDelLock);

//Update maxDeliveryTime
    if(tDeliveryTime > maxDeliveryTime){

    	rc=pthread_mutex_lock(&maxDelLock);
    	while(!changemaxDeliveryTime)
    		rc = pthread_cond_wait(&maxDelWait, &maxDelLock);
    	changemaxDeliveryTime =0;
	
    	maxDeliveryTime = tDeliveryTime;
	
    	changemaxDeliveryTime=1;
    	rc = pthread_cond_signal(&maxDelWait);
    	rc = pthread_mutex_unlock(&maxDelLock);
	}

	tColdPizzaTime = tPizzaDelivered - tReadyPizza; //χρόνος κρυώματος πίτσας

//Update sumColdPizzaTime
    rc=pthread_mutex_lock(&sumColdPizzaLock);
    while(!changesumColdPizzaTime)
    	rc = pthread_cond_wait(&sumColdPizzaWait, &sumColdPizzaLock);
    changesumColdPizzaTime =0;

    sumColdPizzaTime+=tColdPizzaTime;

    changesumColdPizzaTime=1;
    rc = pthread_cond_signal(&sumColdPizzaWait);
    rc = pthread_mutex_unlock(&sumColdPizzaLock);

//Update maxColdPizzaTime
    if(tColdPizzaTime > maxColdPizzaTime){

    	rc=pthread_mutex_lock(&maxColdPizzaLock);
    	while(!changemaxColdPizzaTime)
    		rc = pthread_cond_wait(&maxColdPizzaWait, &maxColdPizzaLock);
    	changemaxColdPizzaTime =0;
	
    	maxColdPizzaTime = tColdPizzaTime; 
	
    	changemaxColdPizzaTime=1;
    	rc = pthread_cond_signal(&maxColdPizzaWait);
    	rc = pthread_mutex_unlock(&maxColdPizzaLock);
	}

	pthread_exit(NULL);

}//Order completion


//main
int main(int argc, char **argv)
{
	
    //Έλεγχος έγκυρου αριθμού παραμέτρων
    if (argc != 3)
    {
        printf ("Unexpected number of arguments\n");
        exit(-1);
    }

    
    Ncust = atoi(argv[1]);
    seed= atoi(argv[2]);
	printf(" Number of customers is %d and seed is %d \n", Ncust, seed);

    //Έλεγχος έγκυρου αριθμού παραμέτρων
    if (Ncust<0) {
        printf("Error, the number of customers should be positive number. Number given %d. \n" , Ncust);
        exit(-1);
    }
 
    //Αρχικοποίηση μεταβλητών και mutexes
    pthread_t tid[Ncust];
    int oid[Ncust];
    int i;
    int rc;
    
    pthread_mutex_init(&orderLock, NULL);
    pthread_mutex_init(&cookLock, NULL);
    pthread_mutex_init(&ovenLock, NULL);
    pthread_mutex_init(&packerLock, NULL);
    pthread_mutex_init(&delivererLock, NULL);
    pthread_mutex_init(&moneyLock, NULL);
    pthread_mutex_init(&screenLock, NULL);

    pthread_mutex_init(&sTransactionLock, NULL);
    pthread_mutex_init(&fTransactionLock, NULL);
    pthread_mutex_init(&sumOrderLock, NULL);
    pthread_mutex_init(&maxOrderLock, NULL);
    pthread_mutex_init(&sumDelLock, NULL);
    pthread_mutex_init(&maxDelLock, NULL);
    pthread_mutex_init(&sumColdPizzaLock, NULL);
    pthread_mutex_init(&maxColdPizzaLock, NULL);

	pthread_cond_init(&orderWait, NULL);
	pthread_cond_init(&cookWait, NULL);
	pthread_cond_init(&ovenWait, NULL);
	pthread_cond_init(&packerWait, NULL);
	pthread_cond_init(&delivererWait, NULL);
	pthread_cond_init(&moneyWait, NULL);
	pthread_cond_init(&screenWait, NULL);

	pthread_cond_init(&sTransactionWait, NULL);
	pthread_cond_init(&fTransactionWait, NULL);
	pthread_cond_init(&sumOrderWait, NULL);
	pthread_cond_init(&maxOrderWait, NULL);
	pthread_cond_init(&sumDelWait, NULL);
	pthread_cond_init(&maxDelWait, NULL);
	pthread_cond_init(&sumColdPizzaWait, NULL);
	pthread_cond_init(&maxColdPizzaWait, NULL);

    //creating threads
    for ( i = 0; i < Ncust; i++) {
    	oid[i]=i+1;
        rc = pthread_create(&tid[i], NULL, order, &oid[i]);	
        if (rc){
			printf("ERROR code from pthread_create() is %d\n", rc);
			exit(-1);
		} 
    }	

    //waiting for threads
    for (i = 0; i < Ncust; i++){
        pthread_join(tid[i], NULL);
    }

    //Printarei τα αποτελέσματα
	printf(" Money: %d $ \n", totalMoney);
	
	printf(" Plain Pizzas: %d \n", amountTotalPlain);
	printf(" Special Pizzas: %d \n", amountTotalSpecial);
	
	printf(" Total Pizzas sold: %d \n", amountTotalPlain + amountTotalSpecial);
	
	printf(" Suceeded Transactions: %d \n", suceededtransaction);
	printf(" Failed Transactions: %d \n", failedtransaction);

	printf(" Average Time for Delivery: %d lepta \n", sumDeliveryTime/(Ncust-failedtransaction));
	printf(" Maximum Time for Delivery: %d lepta \n", maxDeliveryTime);

	printf(" Average Time that the Pizzas got Cold: %d lepta \n", sumColdPizzaTime/(Ncust-failedtransaction));
	printf(" Maximum Time that the Pizzas got Cold: %d lepta \n", maxColdPizzaTime);

    //Αποδέσμευση mutexes 
    pthread_mutex_destroy(&orderLock);
    pthread_mutex_destroy(&cookLock);
    pthread_mutex_destroy(&ovenLock);
    pthread_mutex_destroy(&packerLock);
    pthread_mutex_destroy(&delivererLock);
    pthread_mutex_destroy(&moneyLock);
    pthread_mutex_destroy(&screenLock);

    pthread_mutex_destroy(&sTransactionLock);
    pthread_mutex_destroy(&fTransactionLock);
    pthread_mutex_destroy(&sumOrderLock);
    pthread_mutex_destroy(&maxOrderLock);
    pthread_mutex_destroy(&sumDelLock);
    pthread_mutex_destroy(&maxDelLock);
    pthread_mutex_destroy(&sumColdPizzaLock);
    pthread_mutex_destroy(&maxColdPizzaLock);

    pthread_cond_destroy(&orderWait);
    pthread_cond_destroy(&cookWait);
    pthread_cond_destroy(&ovenWait);
    pthread_cond_destroy(&packerWait);
    pthread_cond_destroy(&delivererWait);
    pthread_cond_destroy(&moneyWait);
    pthread_cond_destroy(&screenWait);

    pthread_cond_destroy(&sTransactionWait);
    pthread_cond_destroy(&fTransactionWait);
    pthread_cond_destroy(&sumOrderWait);
    pthread_cond_destroy(&maxOrderWait);
    pthread_cond_destroy(&sumDelWait);
    pthread_cond_destroy(&maxDelWait);
    pthread_cond_destroy(&sumColdPizzaWait);
    pthread_cond_destroy(&maxColdPizzaWait);

 	return 0;
 }
