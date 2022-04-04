#include <stdint.h>


/* Definition taken from linux/ethtool.h */
#define ETHTOOL_FWVERS_LEN  32
#define ETHTOOL_BUSINFO_LEN 32
#define DRIVER_NAME_LEN 32

#define nic_type_net_ixgbe 1
#define nic_type_net_mlx5 2
#define nic_type_undefined 3

/* these strings are set to whatever the driver author decides... */
 struct ethtool_drvinfo {
  uint32_t cmd;
  char driver[32]; /* driver short name, "tulip", "eepro100" */
  char version[32];    /* driver version string */
  char fw_version[ETHTOOL_FWVERS_LEN]; /* firmware version string */
  char bus_info[ETHTOOL_BUSINFO_LEN];  /* Bus info for this IF. */
  /* For PCI devices, use pci_name(pci_dev). */
  char reserved1[32];
  char reserved2[12];
  /*
  * Some struct members below are filled in
  * using ops->get_sset_count().  Obtaining
  * this info from ethtool_drvinfo is now
  * deprecated; Use ETHTOOL_GSSET_INFO
  * instead.
  */
  uint32_t n_priv_flags;   /* number of flags valid in ETHTOOL_GPFLAGS */
  uint32_t n_stats;    /* number of u64's from ETHTOOL_GSTATS */
  uint32_t testinfo_len;
  uint32_t eedump_len; /* Size of data from ETHTOOL_GEEPROM (bytes) */
  uint32_t regdump_len;    /* Size of data from ETHTOOL_GREGS (bytes) */
  }; 

int rte_ethtool_get_drv_type(uint16_t port_id, int* type);
