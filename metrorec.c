include <pthread.h>
#include <stdio.h>

struct estacao {
  pthread_mutex_t mutex;
  pthread_cond_t vagao_disponivel;
  pthread_cond_t embarque_finalizado;

  int vagao_waiting; //ver se o danado do vagão realmente tá lá
  int assentos_vazios;
//Diferenciar entre quem espera e quem vai embarcar
  int passageiros_aguardando;
  int passageiros_embarque;
  
};

void estacao_init(struct estacao *estacao) {
  pthread_mutex_init(&(estacao->mutex), NULL);
  pthread_cond_init(&(estacao->vagao_disponivel), NULL);
  pthread_cond_init(&(estacao->embarque_finalizado), NULL);
  estacao->assentos_vazios = 0;
  estacao->passageiros_aguardando = 0;
  estacao->passageiros_embarque = 0;
  estacao->vagao_waiting = 0;
}

void estacao_preencher_vagao(struct estacao *estacao, int assentos) {
  pthread_mutex_lock(&(estacao->mutex));
  estacao->vagao_waiting = 1;
  estacao->assentos_vazios = assentos;
  
  if (estacao->passageiros_aguardando > assentos) {
        estacao->passageiros_embarque = assentos;
    } else {
        estacao->passageiros_embarque = estacao->passageiros_aguardando;
  }
  //Libera os passageiros_aguardando
  pthread_cond_broadcast(&(estacao->vagao_disponivel));
  
  while (estacao->passageiros_aguardando != 0 && estacao->assentos_vazios != 0) {
    // Vagão esperando por passageiros_aguardando
    pthread_cond_wait(&(estacao->embarque_finalizado), &(estacao->mutex));
  }
  estacao->vagao_waiting = 0;
  pthread_mutex_unlock(&estacao->mutex);

  return;
}

void estacao_espera_pelo_vagao(struct estacao *estacao) {
  pthread_mutex_lock(&(estacao->mutex));

  estacao->passageiros_aguardando++;

    while (estacao->vagao_waiting == 0 || estacao->passageiros_embarque == 0) { 
        pthread_cond_wait(&estacao->vagao_disponivel, &estacao->mutex);
    }

    estacao->passageiros_embarque--;
  
  pthread_mutex_unlock(&(estacao->mutex));

  return;
}

void estacao_embarque(struct estacao *estacao) {
  pthread_mutex_lock(&(estacao->mutex));

  while (estacao->vagao_waiting == 0) { //pfv espera esse vagão chegar

    pthread_cond_wait(&(estacao->vagao_disponivel), &(estacao->mutex));
  }
  estacao->passageiros_aguardando--;
  estacao->assentos_vazios--;
  pthread_cond_signal(&(estacao->embarque_finalizado));
  pthread_mutex_unlock(&(estacao->mutex));

  return;
}
