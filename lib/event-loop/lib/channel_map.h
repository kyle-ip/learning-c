#ifndef CHANNEL_MAP_H
#define CHANNEL_MAP_H


#include "channel.h"

/**
 * channel 映射表, key 为对应的 socket 描述字（保存了描述字到 channel 的映射）
 * 封装为数组，数组的下标即为描述字，数组的元素为 channel 对象的地址，比如获取描述字 3 的 channel：
 *      struct channel * channel = map->entries[3];
 *
 * event_dispatcher 在获得活动事件列表之后，通过文件描述字找到对应的 channel，从而回调 channel 上的事件处理函数 event_read_callback 和 event_write_callback
 * 而需要回调 channel 上的读、写函数时，调用 channel_event_activate 即可
 */
struct channel_map {
    void **entries;

    /* 描述字数组的长度 */
    /* The number of entries available in entries */
    int nentries;
};


int map_make_space(struct channel_map *map, int slot, int msize);

void map_init(struct channel_map *map);

void map_clear(struct channel_map *map);

#endif