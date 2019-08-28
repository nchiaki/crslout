
/* 
 * キューデータ取り扱いマクロ
 *
 * 2014.10.17 at first from NORTi
 * 2012/10/20 chiaki	first release
 */

#ifndef	queue_inc
#define queue_inc

typedef struct	queue {	/* キュー領域定義 */
	struct queue	*que_next;
	struct queue	*que_back;
} QUE;

typedef	QUE	*QLINK;

/* キューの初期化 */
#define	QINIT(que)	(((QLINK)(que))->que_next = (((QLINK)que))->que_back = (QLINK)(que))

/* 前キューの取得 */
#define	NEXT(que)	(((QLINK)(que))->que_next)

/* 後キューの取得 */
#define	BACK(que)	(((QLINK)(que))->que_back)

/* キューデータの抜き出し */
#define	QREMOVE(que)	{BACK(que)->que_next = NEXT(que); NEXT(que)->que_back = BACK(que); QINIT(que); }

/* キューデータの追加 */
#define	QAPPEND(pre, que)	\
	{BACK(que) = (QLINK)pre; NEXT(que) = NEXT(pre); NEXT(pre) = (QLINK)que; NEXT(que)->que_back = (QLINK)que;}

/* キューデータの挿入 */
#define	QINSERT(que, aft) \
	{NEXT(que) = (QLINK)aft; BACK(que) = BACK(aft); BACK(aft) = (QLINK)que; BACK(que)->que_next = (QLINK)que;}

/* ルートキューへのエンキュー */
#define	ENQUEUE(root, que)	\
	{BACK(que) = BACK(root); BACK(root)->que_next = (QLINK)(que); NEXT(que) = (root); BACK(root) = (QLINK)(que);}

/* ルートキューからのデキュー */
//#define	DEQUEUE(que, root)	{(QLINK)(que) = NEXT(root); QREMOVE(que);}		/*gcc4以降で左辺値のキャスト禁止のためコンパイルが通らない*/
//#define	DEQUEUE(que, root)	{(que) = NEXT(root); QREMOVE(que);}
#define	DEQUEUE(que, cast, root)	{que = cast NEXT(root); QREMOVE(que);}

/* ルートキューへのエンキュー状態 */
#define	QisEMPTY(root)	((NEXT(root)==(root)) && (BACK(root)==(root)))

/* ルートキューの連結 */
#define	QJOIN(dstrt, srcrt)	{QUE	*mstbck, *nwtop, *nwend;\
			nwtop = (srcrt)->que_next; nwend = (srcrt)->que_back;\
			mstbck = (dstrt)->que_back;\
			mstbck->que_next = nwtop;\
			nwtop->que_back = mstbck;\
			(dstrt)->que_back = nwend;\
			nwend->que_next = (dstrt);\
			QINIT(srcrt);\
}

#endif /* queue_inc */
