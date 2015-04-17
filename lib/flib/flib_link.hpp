/**
 * @file
 * @author Dirk Hutter <hutter@compeng.uni-frankfurt.de>
 * @author Dominic Eschweiler<dominic.eschweiler@cern.ch>
 *
 */

#ifndef FLIB_LINK_HPP
#define FLIB_LINK_HPP

#include <pda/device.hpp>
#include <pda/dma_buffer.hpp>
#include <pda/pci_bar.hpp>

#include <registers.h>
#include <data_structures.hpp>
#include <register_file_bar.hpp>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"

using namespace pda;

namespace flib {
// Tags to indicate mode of buffer initialization
struct create_only_t {};
struct open_only_t {};
struct register_only_t {};

static const create_only_t create_only = create_only_t();
static const open_only_t open_only = open_only_t();
static const register_only_t register_only = register_only_t();

class dma_channel;
class register_file_bar;

class flib_link {

public:
  flib_link(size_t link_index, device* dev, pci_bar* bar);
  ~flib_link();

  void init_dma(void* data_buffer,
                size_t data_buffer_log_size,
                void* desc_buffer,
                size_t desc_buffer_log_size);

  void deinit_dma();

  /*** Configuration and control ***/

  /**
   * REG: mc_gen_cfg
   * bit 0 set_start_index
   * bit 1 rst_pending_mc
   * bit 2 packer enable
   */
  void set_start_idx(uint64_t index);
  void rst_pending_mc();
  void rst_cnet_link();
  void enable_cbmnet_packer(bool enable);
  void enable_cbmnet_packer_debug_mode(bool enable);

  /*** CBMnet control interface ***/

  int send_dcm(const struct ctrl_msg* msg);
  int recv_dcm(struct ctrl_msg* msg);

  /**
   * RORC_REG_DLM_CFG 0 set to send (global reg)
   * RORC_REG_GTX_DLM 3..0 tx type, 4 enable,
   * 8..5 rx type, 31 set to clear rx reg
   */
  void prepare_dlm(uint8_t type, bool enable);
  void send_dlm();
  uint8_t recv_dlm();

  // REG: datapath_cfg
  // bit 1-0 data_sel (10: link, 11: pgen, 01: emu, 00: disable)
  enum data_sel_t { rx_disable, rx_emu, rx_link, rx_pgen };
  void set_data_sel(data_sel_t rx_sel);
  void set_hdr_config(const struct hdr_config* config);

  uint64_t pending_mc();
  uint64_t mc_index();
  data_sel_t data_sel();

  size_t link_index() { return m_link_index; };
  pda::device* parent_device() { return m_parent_device; };

  dma_channel* channel() const;
  register_file* register_file_packetizer() const { return m_rfpkt.get(); }
  register_file* register_file_gtx() const { return m_rfgtx.get(); }

  
  struct link_status_t {
    bool link_active;
    bool data_rx_stop;
    bool ctrl_rx_stop;
    bool ctrl_tx_stop;
  };

  struct link_status_t link_status();

  struct diag_flags_t {
    bool pcs_startup;
    bool ebtb_code_err;
    bool ebtb_disp_err;
    bool crc_error;
    bool packet;
    bool packet_err;
    bool rx_clk_stable;
    bool tx_clk_stable;
    bool ebtb_detect;
    bool serdes_ready;
    bool link_active;
  };

  // read counters
  uint32_t diag_pcs_startup();
  uint32_t diag_ebtb_code_err();
  uint32_t diag_ebtb_disp_err();
  uint32_t diag_crc_error();
  uint32_t diag_packet();
  uint32_t diag_packet_err();
  // read all flags
  diag_flags_t diag_flags();
  // clear all counters
  void diag_clear();


protected:
  std::unique_ptr<dma_channel> m_dma_channel;
  std::unique_ptr<register_file> m_rfglobal; // TODO remove this later
  std::unique_ptr<register_file> m_rfpkt;
  std::unique_ptr<register_file> m_rfgtx;

  size_t m_link_index = 0;

  sys_bus_addr m_base_addr;
  pda::device* m_parent_device;


  void init_datapath();
  void reset_datapath();

};
}

#pragma GCC diagnostic pop

#endif /** FLIB_LINK_HPP */
