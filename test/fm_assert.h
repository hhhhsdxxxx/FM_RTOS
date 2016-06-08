#ifndef _FM_ASSERT_H_
#define _FM_ASSERT_H_

/*
 * 当第一个参数为真（值为1），则什么都不做
 * 当地一个参数为假（值为0），则串口输出字符串，并停在该处，永不跳出
 */
void fm_assert(int, const char*);

#endif
