#ifndef PROCESS_MGR_H
#define PROCESS_MGR_H

#include "core/traveler.h"

void spawn_travelers(Traveler* travelers, int n);
void wait_for_all_travelers(Traveler* travelers, int n);

#endif // PROCESS_MGR_H