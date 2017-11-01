/*
 * TU Dresden, RNP 2017 Exercise 2
 * Creating our own Layer 3 protocol and send it over raw sockets.
 * Copyright 2017 Dominik Pataky <dominik.pataky@tu-dresden.de>
 * Licensed under GPLv3, feel free to use and expand it. See LICENSE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_ether.h>  // includes some definitions
#include <linux/if_packet.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <unistd.h>

#define SPARK_PROTO 0x666
#define SPARK_VERSION 0x1
#define SPARK_HLEN 3
#define SPARK_DLEN ETH_DATA_LEN-SPARK_HLEN

// Spark frame with header and data payload
struct sparkframe {
    uint8_t version;
    uint16_t data_size;
    unsigned char data[SPARK_DLEN];
};

// Ethernet frame, usable either with struct fields (.frame) or binary stream (.binary)
union etherframe {
    struct {
        struct ethhdr header;   // h_dest, h_source, h_proto
        struct sparkframe spark;
    } frame;
    unsigned char binary[ETH_FRAME_LEN];
};

int main(int argc, char *argv[]) {
    // We want three input arguments
    if (argc < 4) {
        printf("Usage: %s <MAC address> <interface> <message>", argv[0]);
        return 1;
    }

    char *dest_mac = argv[1];
    char *iface = argv[2];
    char *data = argv[3];
    uint16_t data_size = strlen(data);

    union etherframe packet;

    // Create the socket as raw socket with the SPARK_PROTO identifier
    int sock;
    if ((sock = socket(AF_PACKET, SOCK_RAW, htons(SPARK_PROTO))) < 0) {
        printf("Error while opening raw socket, error %d!\n", sock);
        return 2;
    };

    // Zero the query buffer for ioctl
    struct ifreq query_buffer;
    memset(&query_buffer, 0x00, sizeof(query_buffer));

    // Insert interface name into buffer
    strncpy(query_buffer.ifr_name, iface, IFNAMSIZ);

    // Make query via ioctl to get interface index
    int ifindex;
    if (ioctl(sock, SIOCGIFINDEX, &query_buffer) < 0) {
        printf("Error while getting interface index!\n");
        close(sock);
        return 3;
    }
    ifindex = query_buffer.ifr_ifindex;

    // Query the source MAC address via ioctl
    unsigned char source_mac[ETH_ALEN];
    if (ioctl(sock, SIOCGIFHWADDR, &query_buffer) < 0) {
        printf("Error while getting interface source MAC!\n");
        close(sock);
        return 4;
    }
    memcpy((void*)source_mac, (void*)(query_buffer.ifr_hwaddr.sa_data), ETH_ALEN);

    // Fill the frame

    // Read the destination MAC from CLI input
    int _byte = 0;
    for (int i = 0; i < strlen(dest_mac); i=i+3) {
        // Take 3 bytes e.g. "d3:" and read 16 bits, dropping the semicolon
        packet.frame.header.h_dest[_byte] = strtol(&dest_mac[i], NULL, 16);
        _byte++;
    }

    // Ethernet frame header
    memcpy(packet.frame.header.h_source, source_mac, ETH_ALEN);
    packet.frame.header.h_proto = htons(SPARK_PROTO);

    // Spark frame header
    packet.frame.spark.version = SPARK_VERSION;
    packet.frame.spark.data_size = data_size;
    memcpy(packet.frame.spark.data, data, data_size);

    // Calculate the real length of the packet
    unsigned int frame_size = ETH_HLEN + SPARK_HLEN + data_size;

    // Socket internals, taken from tutorial
    struct sockaddr_ll saddrll;
    memset((void*)&saddrll, 0, sizeof(saddrll));
    saddrll.sll_family = PF_PACKET;
    saddrll.sll_ifindex = ifindex;
    saddrll.sll_halen = ETH_ALEN;
    memcpy((void*)(saddrll.sll_addr), (void*)dest_mac, ETH_ALEN);

    // Finally, send the assembled packet (using the union as the binary stream)
    int sent;
    if ((sent = sendto(sock, packet.binary, frame_size, 0, (struct sockaddr*)&saddrll, sizeof(saddrll))) > 0) {
        printf("%d bytes successfully sent!\n", sent);
        close(sock);
    } else {
        printf("Error while sending packet!\n");
        close(sock);
        return 5;
    }

    return 0;
}
