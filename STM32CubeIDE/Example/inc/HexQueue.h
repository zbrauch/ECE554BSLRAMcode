#ifndef HEXQUEUE_H_
#define HEXQUEUE_H_

#define HEX_QUEUE_LEN 200

typedef struct {
	uint8_t start; //array index of first element in queue
	uint8_t next; //array index of the next element to write
	uint8_t buf[HEX_QUEUE_LEN];
	uint8_t len;
} HEXQueue;

void HEXQueueInit(HEXQueue *q) {
	q->start = 0;
	q->next = 0;
	q->len = 0;
}

void HEXQueueFixBounds(HEXQueue *q) {
	if(q->start >= HEX_QUEUE_LEN)
		q->start -= HEX_QUEUE_LEN;
	if(q->next >= HEX_QUEUE_LEN)
		q->next -= HEX_QUEUE_LEN;
	if(q->len > HEX_QUEUE_LEN)
		q->len = HEX_QUEUE_LEN;
}

void HEXQueueAdd(HEXQueue *q, uint8_t element) {
	q->buf[q->next] = element;
	q->next++;
	q->len++;

	if(q->len == HEX_QUEUE_LEN)
		q->start++;

	HEXQueueFixBounds(q);
}

void HEXQueueAddArray(HEXQueue *q, uint8_t *inbuf, uint8_t len) {
	for(uint8_t i = 0; i < len; i++)
		HEXQueueAdd(q, inbuf[i]);
}

uint8_t HEXQueueGetIdx(HEXQueue *q, uint8_t idx) {
	uint8_t retIdx = (idx + q->start) % HEX_QUEUE_LEN;
	return q->buf[retIdx];
}


//pulls an Intel hex command out of the buffer if one exists
	//returns 1 if command extracted, 0 if none found
uint8_t HEXQueueExtractHex(HEXQueue *q, uint8_t *outBuf) {
	for(uint8_t i = 0; i < q->len - 5; i++) { //scan where queue is full enough for minimum len hex command
		if(HEXQueueGetIdx(q, i) == ':') {
			//check if complete hex command
			uint8_t dataLen = HEXQueueGetIdx(q, i+1);
			if((q->len - i) >= 5 + dataLen) { //if hex fully received
				//check checksum
				uint8_t checksum = 0;
				for(uint8_t j = 1; j < dataLen + 5; j++) {
					checksum += HEXQueueGetIdx(q, i+j);
				}
				checksum = (~checksum)+1; //2's compliment
				if(checksum != HEXQueueGetIdx(q, i+dataLen+5)) {
					continue;
				}

				//copy hex command to out buffer
				for(uint8_t j = 1; j < dataLen + 6; j++)
					outBuf[j-1] = HEXQueueGetIdx(q, i+j);
				//return dataLen+6;

				//move start of buffer to end of hex
				q->len -= i+5+dataLen;
				q->start = (q->start+i+dataLen+5) % HEX_QUEUE_LEN;
				q->next = (q->start+q->len) % HEX_QUEUE_LEN;
				return 1;
			}
		}
	}
	return 0;
}

#endif /* HEXQUEUE_H_ */
