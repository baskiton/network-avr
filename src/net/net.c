#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "net/net.h"
#include "net/ether.h"

/*!
 * @brief Allocate a network buffer for rx on a specific device
 * @param dev Network device to receive
 * @param size Size to allocate (for ethernet is eth header, data, padding, crc)
 * @return Pointer to buffer or NULL if not enough memory
 */
struct net_buff_s *net_alloc_buff(struct net_dev_s *net_dev, uint16_t size) {
    struct net_buff_s *buff;
    uint8_t *head;

    buff = malloc(sizeof(struct net_buff_s));
    if (!buff)
        goto end;
    
    head = malloc(size);
    if (!head) {
        free(buff);
        buff = NULL;
        goto end;
    }
    
    memset(buff, 0, sizeof(struct net_buff_s));

    buff->head = head;
    buff->data = head;
    buff->mac_header = buff->network_header = buff->transport_header = (uint8_t)-1U;
    buff->net_dev = net_dev;

end:
    return buff;
}

/*!
 * @brief Free the allocating memory an Net Buffer
 */
void net_free_buff(struct net_buff_s *buff) {
    free(buff->head);
    free(buff);
}