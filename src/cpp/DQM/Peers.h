#ifndef DQM_PEERS_H
# define DQM_PEERS_H

# include "DQM/Objects.h"
# include "classlib/iobase/Socket.h"

static const uint32_t PEER_HELLO		= 0;
static const uint32_t PEER_LIST_OBJECTS		= 1;
static const uint32_t PEER_GET_OBJECT		= 2;
static const uint32_t PEER_GET_IMAGE		= 3;
static const uint32_t PEER_UPDATE_ME		= 4;

static const uint32_t PEER_LIST_BEGIN		= 100;
static const uint32_t PEER_LIST_END		= 101;
static const uint32_t PEER_OBJECT		= 102;
static const uint32_t PEER_NONE			= 103;
static const uint32_t PEER_IMAGE		= 104;

struct Bucket
{
  Bucket	*next;
  DataBlob	data;
};

struct PeerInfo
{
  std::string	peeraddr;
  lat::Socket	*peer;
  DataBlob	incoming;
  Bucket	*sendq;
  size_t	sendpos;
  unsigned	mask;
  bool		update;
  bool		updated;
};

#endif // DQM_PEERS_H
