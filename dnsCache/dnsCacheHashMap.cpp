#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define TABLE_SIZE 128
#define MAX_IPS 5
#define MAX_DOMAIN_LEN 256
#define MAX_CNAME_DEPTH 5

typedef enum { RECORD_A, RECORD_CNAME } record_type_t;

typedef struct dns_record {
    char domain[MAX_DOMAIN_LEN];
    record_type_t type;

    // A record
    char ips[MAX_IPS][16];
    int ip_count;

    // CNAME
    char cname[MAX_DOMAIN_LEN];

    time_t expiry;

    struct dns_record* next;
} dns_record_t;

dns_record_t* dns_table[TABLE_SIZE] = {0};

/* ---------------- Hash Function ---------------- */
unsigned int hash(const char* str) {
    unsigned int h = 0;
    while (*str)
        h = (h * 31) + *str++;
    return h % TABLE_SIZE;
}

/* ---------------- Insert A Record ---------------- */
void insert_a_record(const char* domain, char ips[][16], int ip_count, int ttl_seconds) {
    unsigned int index = hash(domain);

    dns_record_t* rec = malloc(sizeof(dns_record_t));
    if (!rec)
        return;

    memset(rec, 0, sizeof(dns_record_t));
    strncpy(rec->domain, domain, MAX_DOMAIN_LEN - 1);
    rec->type = RECORD_A;
    rec->ip_count = ip_count > MAX_IPS ? MAX_IPS : ip_count;

    for (int i = 0; i < rec->ip_count; i++)
        strncpy(rec->ips[i], ips[i], 15);

    rec->expiry = time(NULL) + ttl_seconds;

    rec->next = dns_table[index];
    dns_table[index] = rec;
}

/* ---------------- Insert CNAME ---------------- */
void insert_cname_record(const char* domain, const char* cname, int ttl_seconds) {
    unsigned int index = hash(domain);

    dns_record_t* rec = malloc(sizeof(dns_record_t));
    if (!rec)
        return;

    memset(rec, 0, sizeof(dns_record_t));
    strncpy(rec->domain, domain, MAX_DOMAIN_LEN - 1);
    rec->type = RECORD_CNAME;
    strncpy(rec->cname, cname, MAX_DOMAIN_LEN - 1);
    rec->expiry = time(NULL) + ttl_seconds;

    rec->next = dns_table[index];
    dns_table[index] = rec;
}

/* ---------------- Find Record ---------------- */
dns_record_t* find_record(const char* domain) {
    unsigned int index = hash(domain);
    dns_record_t* curr = dns_table[index];

    while (curr) {
        if (strcmp(curr->domain, domain) == 0)
            return curr;
        curr = curr->next;
    }
    return NULL;
}

/* ---------------- Resolve (Recursive) ---------------- */
int resolve(const char* domain, int depth) {
    if (depth > MAX_CNAME_DEPTH) {
        printf("CNAME loop detected\n");
        return 0;
    }

    dns_record_t* rec = find_record(domain);
    if (!rec) {
        printf("No record found for %s\n", domain);
        return 0;
    }

    if (time(NULL) > rec->expiry) {
        printf("Record expired for %s\n", domain);
        return 0;
    }

    if (rec->type == RECORD_A) {
        printf("A record for %s:\n", domain);
        for (int i = 0; i < rec->ip_count; i++)
            printf("  %s\n", rec->ips[i]);
        return 1;
    }

    // CNAME case
    printf("%s is CNAME for %s\n", domain, rec->cname);
    return resolve(rec->cname, depth + 1);
}

/* ---------------- Free Memory ---------------- */
void cleanup() {
    for (int i = 0; i < TABLE_SIZE; i++) {
        dns_record_t* curr = dns_table[i];
        while (curr) {
            dns_record_t* tmp = curr;
            curr = curr->next;
            free(tmp);
        }
    }
}

/* ---------------- Main ---------------- */
int main() {
    char ips[][16] = {"1.1.1.1", "1.1.1.2"};

    insert_a_record("server.com", ips, 2, 60);
    insert_cname_record("www.server.com", "server.com", 60);

    resolve("www.server.com", 0);

    cleanup();
    return 0;
}
