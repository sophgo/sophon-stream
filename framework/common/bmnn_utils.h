#pragma once

#include <assert.h>

#include <cstring>
#include <iostream>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>

#include "bmruntime_interface.h"
#include "no_copyable.h"

extern "C" {
extern bm_status_t bm_thread_sync_from_core(bm_handle_t handle, int core_id) __attribute__((weak));
}

class BMNNTensor {
  /**
   *  members from bm_tensor {
   *  bm_data_type_t dtype;
   bm_shape_t shape;
   bm_device_mem_t device_mem;
   bm_store_mode_t st_mode;
   }
   */
  bm_handle_t m_handle;

  std::string m_name;
  float* m_cpu_data;
  float m_scale;
  bm_tensor_t* m_tensor;

  bool can_mmap;

 public:
  BMNNTensor(bm_handle_t handle, const char* name, float scale,
             bm_tensor_t* tensor, bool can_mmap)
      : m_handle(handle),
        m_name(name),
        m_cpu_data(nullptr),
        m_scale(scale),
        m_tensor(tensor),
        can_mmap(can_mmap) {}

  virtual ~BMNNTensor() {
    if (m_cpu_data == NULL) return;
    if (can_mmap && BM_FLOAT32 == m_tensor->dtype) {
      int tensor_size = bm_mem_get_device_size(m_tensor->device_mem);
      bm_status_t ret =
          bm_mem_unmap_device_mem(m_handle, m_cpu_data, tensor_size);
      assert(BM_SUCCESS == ret);
    } else {
      delete[] m_cpu_data;
    }
  }

  int set_device_mem(bm_device_mem_t* mem) {
    this->m_tensor->device_mem = *mem;
    return 0;
  }

  const bm_device_mem_t* get_device_mem() {
    return &this->m_tensor->device_mem;
  }

  float cpu_half2float(unsigned short x) {
      unsigned sign = ((x >> 15) & 1);
      unsigned exponent = ((x >> 10) & 0x1f);
      unsigned mantissa = ((x & 0x3ff) << 13);
      if (exponent == 0x1f) {  /* NaN or Inf */
          mantissa = (mantissa ? (sign = 0, 0x7fffff) : 0);
          exponent = 0xff;
      } else if (!exponent) {  /* Denorm or Zero */
          if (mantissa) {
              unsigned int msb;
              exponent = 0x71;
              do {
                  msb = (mantissa & 0x400000);
                  mantissa <<= 1;  /* normalize */
                  --exponent;
              } while (!msb);
              mantissa &= 0x7fffff;  /* 1.mantissa is implicit */
          }
      } else {
          exponent += 0x70;
      }
      int temp = ((sign << 31) | (exponent << 23) | mantissa);
  
      return *((float*)((void*)&temp));
  }

  float* get_cpu_data() {
    if (m_cpu_data) return m_cpu_data;
    bm_status_t ret;
    float* pFP32 = nullptr;
    int count = bmrt_shape_count(&m_tensor->shape);
    // in SOC mode, device mem can be mapped to host memory, faster then using
    // d2s
    if (can_mmap) {
      if (m_tensor->dtype == BM_FLOAT32) {
        unsigned long long addr;
        ret = bm_mem_mmap_device_mem(m_handle, &m_tensor->device_mem, &addr);
        assert(BM_SUCCESS == ret);
        ret = bm_mem_invalidate_device_mem(m_handle, &m_tensor->device_mem);
        assert(BM_SUCCESS == ret);
        pFP32 = (float*)addr;
      } else if (BM_INT8 == m_tensor->dtype) {
        int8_t* pI8 = nullptr;
        unsigned long long addr;
        ret = bm_mem_mmap_device_mem(m_handle, &m_tensor->device_mem, &addr);
        assert(BM_SUCCESS == ret);
        ret = bm_mem_invalidate_device_mem(m_handle, &m_tensor->device_mem);
        assert(BM_SUCCESS == ret);
        pI8 = (int8_t*)addr;

        // dtype convert
        pFP32 = new float[count];
        assert(pFP32 != nullptr);
        for (int i = 0; i < count; ++i) {
          pFP32[i] = pI8[i] * m_scale;
        }
        ret = bm_mem_unmap_device_mem(
            m_handle, pI8, bm_mem_get_device_size(m_tensor->device_mem));
        assert(BM_SUCCESS == ret);
      } else if (m_tensor->dtype == BM_INT32) {
        int32_t* pI32 = nullptr;
        unsigned long long addr;
        ret = bm_mem_mmap_device_mem(m_handle, &m_tensor->device_mem, &addr);
        assert(BM_SUCCESS == ret);
        ret = bm_mem_invalidate_device_mem(m_handle, &m_tensor->device_mem);
        assert(BM_SUCCESS == ret);
        pI32 = (int32_t*)addr;
        // dtype convert
        pFP32 = new float[count];
        assert(pFP32 != nullptr);
        for (int i = 0; i < count; ++i) {
          pFP32[i] = pI32[i] * m_scale;
        }
        ret = bm_mem_unmap_device_mem(
            m_handle, pI32, bm_mem_get_device_size(m_tensor->device_mem));
        assert(BM_SUCCESS == ret);
      } else if (m_tensor->dtype == BM_FLOAT16) {
        unsigned short* pFP16 = nullptr;
        unsigned long long addr;
        ret = bm_mem_mmap_device_mem(m_handle, &m_tensor->device_mem, &addr);
        assert(BM_SUCCESS == ret);
        ret = bm_mem_invalidate_device_mem(m_handle, &m_tensor->device_mem);
        assert(BM_SUCCESS == ret);
        pFP16 = (unsigned short*)addr;
        // dtype convert
        pFP32 = new float[count];
        assert(pFP32 != nullptr);
        for (int i = 0; i < count; ++i) {
          pFP32[i] = cpu_half2float(pFP16[i]);
        }
        ret = bm_mem_unmap_device_mem(
            m_handle, pFP16, bm_mem_get_device_size(m_tensor->device_mem));
        assert(BM_SUCCESS == ret);
      } else {
        std::cout << "NOT support dtype=" << m_tensor->dtype << std::endl;
      }
    } else {
      // the common method using d2s
      if (m_tensor->dtype == BM_FLOAT32) {
        pFP32 = new float[count];
        assert(pFP32 != nullptr);
        ret = bm_memcpy_d2s_partial(m_handle, pFP32, m_tensor->device_mem,
                                    count * sizeof(float));
        assert(BM_SUCCESS == ret);
      } else if (BM_FLOAT16 == m_tensor->dtype) {
        unsigned short* pFP16 = nullptr;
        int tensor_size = bmrt_tensor_bytesize(m_tensor);
        pFP16 = new unsigned short[count];
        assert(pFP16 != nullptr);

        // dtype convert
        pFP32 = new float[count];
        assert(pFP32 != nullptr);
        ret = bm_memcpy_d2s_partial(m_handle, pFP16, m_tensor->device_mem,
                                    tensor_size);
        assert(BM_SUCCESS == ret);
        for (int i = 0; i < count; ++i) {
          pFP32[i] = cpu_half2float(pFP16[i]);
        }
        delete[] pFP16;
      } else if (BM_INT8 == m_tensor->dtype) {
        int8_t* pI8 = nullptr;
        int tensor_size = bmrt_tensor_bytesize(m_tensor);
        pI8 = new int8_t[tensor_size];
        assert(pI8 != nullptr);

        // dtype convert
        pFP32 = new float[count];
        assert(pFP32 != nullptr);
        ret = bm_memcpy_d2s_partial(m_handle, pI8, m_tensor->device_mem,
                                    tensor_size);
        assert(BM_SUCCESS == ret);
        for (int i = 0; i < count; ++i) {
          pFP32[i] = pI8[i] * m_scale;
        }
        delete[] pI8;
      } else if (BM_INT32 == m_tensor->dtype) {
        int32_t* pI32 = nullptr;
        int tensor_size = bmrt_tensor_bytesize(m_tensor);
        pI32 = new int32_t[count];
        assert(pI32 != nullptr);

        // dtype convert
        pFP32 = new float[count];
        assert(pFP32 != nullptr);
        ret = bm_memcpy_d2s_partial(m_handle, pI32, m_tensor->device_mem,
                                    tensor_size);
        assert(BM_SUCCESS == ret);
        for (int i = 0; i < count; ++i) {
          pFP32[i] = pI32[i] * m_scale;
        }
        delete[] pI32;
      } else {
        std::cout << "NOT support dtype=" << m_tensor->dtype << std::endl;
      }
    }

    m_cpu_data = pFP32;
    return m_cpu_data;
  }

  const bm_shape_t* get_shape() { return &m_tensor->shape; }

  bm_data_type_t get_dtype() { return m_tensor->dtype; }

  float get_scale() { return m_scale; }

  void set_shape(const int* shape, int dims) {
    m_tensor->shape.num_dims = dims;
    for (int i = 0; i < dims; i++) {
      m_tensor->shape.dims[i] = shape[i];
    }
  }
  void set_shape_by_dim(int dim, int size) {
    assert(m_tensor->shape.num_dims > dim);
    m_tensor->shape.dims[dim] = size;
  }
  int get_num() { return m_tensor->shape.dims[0]; }
};

class BMNNNetwork : public ::sophon_stream::common::NoCopyable {
 public:
  const bm_net_info_t* m_netinfo;
  bm_tensor_t* m_inputTensors;
  // bm_tensor_t* m_outputTensors;
  std::vector<std::shared_ptr<bm_tensor_t>> m_outputTensors;
  bm_handle_t m_handle;
  void* m_bmrt;
  bool is_soc;
  std::set<int> m_batches;
  int m_max_batch;
  size_t max_size = 0;

  std::unordered_map<std::string, bm_tensor_t*> m_mapInputs;
  std::unordered_map<std::string, bm_tensor_t*> m_mapOutputs;

 public:
  BMNNNetwork(void* bmrt, const std::string& name) : m_bmrt(bmrt) {
    m_handle = static_cast<bm_handle_t>(bmrt_get_bm_handle(bmrt));
    m_netinfo = bmrt_get_network_info(bmrt, name.c_str());
    m_max_batch = -1;
    std::vector<int> batches;
    for (int i = 0; i < m_netinfo->stage_num; i++) {
      batches.push_back(m_netinfo->stages[i].input_shapes[0].dims[0]);
      if (m_max_batch < batches.back()) {
        m_max_batch = batches.back();
      }
    }
    m_batches.insert(batches.begin(), batches.end());
    m_inputTensors = new bm_tensor_t[m_netinfo->input_num];
    // m_outputTensors = new bm_tensor_t[m_netinfo->output_num];
    for (int i = 0; i < m_netinfo->output_num; ++i)
      m_outputTensors.push_back(std::make_shared<bm_tensor_t>());
    for (int i = 0; i < m_netinfo->input_num; ++i) {
      m_inputTensors[i].dtype = m_netinfo->input_dtypes[i];
      m_inputTensors[i].shape = m_netinfo->stages[0].input_shapes[i];
      m_inputTensors[i].st_mode = BM_STORE_1N;
      // input device mem should be provided outside, such as from image's
      // contiguous mem
      m_inputTensors[i].device_mem = bm_mem_null();
    }

    for (int i = 0; i < m_netinfo->output_num; ++i) {
      // m_outputTensors[i].dtype = m_netinfo->output_dtypes[i];
      // m_outputTensors[i].shape = m_netinfo->stages[0].output_shapes[i];
      // m_outputTensors[i].st_mode = BM_STORE_1N;
      m_outputTensors[i]->dtype = m_netinfo->output_dtypes[i];
      m_outputTensors[i]->shape = m_netinfo->stages[0].output_shapes[i];
      m_outputTensors[i]->st_mode = BM_STORE_1N;
      // alloc as max size to reuse device mem, avoid to alloc and free
      // everytime
      max_size = 0;
      for (int s = 0; s < m_netinfo->stage_num; s++) {
        size_t out_size =
            bmrt_shape_count(&m_netinfo->stages[s].output_shapes[i]);
        if (max_size < out_size) {
          max_size = out_size;
        }
      }
      if (BM_FLOAT32 == m_netinfo->output_dtypes[i]) max_size *= 4;
      // auto ret =  bm_malloc_device_byte(m_handle,
      // &m_outputTensors[i].device_mem, max_size);
      auto ret = bm_malloc_device_byte_heap(
          m_handle, &m_outputTensors[i]->device_mem, 0, max_size);
      assert(BM_SUCCESS == ret);
    }
    struct bm_misc_info misc_info;
    bm_status_t ret = bm_get_misc_info(m_handle, &misc_info);
    assert(BM_SUCCESS == ret);
    is_soc = misc_info.pcie_soc_mode == 1;

    printf("*** Run in %s mode ***\n", is_soc ? "SOC" : "PCIE");

    // assert(m_netinfo->stage_num == 1);
    showInfo();
  }

  ~BMNNNetwork() {
    // Free input tensors
    delete[] m_inputTensors;
    // Free output tensors
    //  for(int i = 0; i < m_netinfo->output_num; ++i) {
    //    if (m_outputTensors[i].device_mem.size != 0) {
    //      bm_free_device(m_handle, m_outputTensors[i].device_mem);
    //    }
    //  }
    for (int i = 0; i < m_netinfo->output_num; ++i) {
      if (m_outputTensors[i]->device_mem.size != 0) {
        bm_free_device(m_handle, m_outputTensors[i]->device_mem);
      }
    }
    // delete []m_outputTensors;
  }

  int maxBatch() const { return m_max_batch; }
  int get_nearest_batch(int real_batch) {
    for (auto batch : m_batches) {
      if (batch >= real_batch) {
        return batch;
      }
    }
    assert(0);
    return m_max_batch;
  }

  std::shared_ptr<BMNNTensor> inputTensor(int index) {
    assert(index < m_netinfo->input_num);
    return std::make_shared<BMNNTensor>(m_handle, m_netinfo->input_names[index],
                                        m_netinfo->input_scales[index],
                                        &m_inputTensors[index], is_soc);
  }

  int outputTensorNum() { return m_netinfo->output_num; }

  std::shared_ptr<BMNNTensor> outputTensor(int index) {
    assert(index < m_netinfo->output_num);
    // return std::make_shared<BMNNTensor>(m_handle,
    // m_netinfo->output_names[index],
    //     m_netinfo->output_scales[index], &m_outputTensors[index], is_soc);
    return std::make_shared<BMNNTensor>(
        m_handle, m_netinfo->output_names[index],
        m_netinfo->output_scales[index], m_outputTensors[index].get(), is_soc);
  }

  template <bool dual_core = false>
  int forward(std::vector<std::shared_ptr<bm_tensor_t>>& inputTensors,
              std::vector<std::shared_ptr<bm_tensor_t>>& outputTensors) {
    bool user_mem = false;  // if false, bmrt will alloc mem every time.
    // if (m_outputTensors->device_mem.size != 0) {
    //   // if true, bmrt don't alloc mem again.
    //   user_mem = true;
    // }

    if (outputTensors[0]->device_mem.size != 0) {
      // if true, bmrt don't alloc mem again.
      user_mem = true;
    }
    bm_tensor_t temp_inputTensors[m_netinfo->input_num];
    for (int i = 0; i < m_netinfo->input_num; ++i)
      temp_inputTensors[i] = *inputTensors[i];

    bm_tensor_t temp_outputTensors[m_netinfo->output_num];
    for (int i = 0; i < m_netinfo->output_num; ++i)
      temp_outputTensors[i] = *outputTensors[i];

    bool ok = false;
    if constexpr (dual_core) {
      // if true, use double core in inference. support in bm1688
      int core_id = rand() % 2;
      const int* core_list = &core_id;
      ok = bmrt_launch_tensor_multi_cores(
          m_bmrt, m_netinfo->name, temp_inputTensors, m_netinfo->input_num,
          temp_outputTensors, m_netinfo->output_num, user_mem, false, core_list,
          1);
      bool status = bm_thread_sync_from_core(m_handle, core_id);
      assert(BM_SUCCESS == status);
    } else {
      // if false, use single core in inference. support in bm1684 & 1684x &
      // 1688
      ok = bmrt_launch_tensor_ex(m_bmrt, m_netinfo->name, temp_inputTensors,
                                 m_netinfo->input_num, temp_outputTensors,
                                 m_netinfo->output_num, user_mem, false);
      bool status = bm_thread_sync(m_handle);
      assert(BM_SUCCESS == status);
    }

    if (!ok) {
      std::cout << "bm_launch_tensor() failed=" << std::endl;
      return -1;
    }

    return 0;
  }

  static std::string shape_to_str(const bm_shape_t& shape) {
    std::string str = "[ ";
    for (int i = 0; i < shape.num_dims; i++) {
      str += std::to_string(shape.dims[i]) + " ";
    }
    str += "]";
    return str;
  }

  void showInfo() {
    const char* dtypeMap[] = {
        "FLOAT32", "FLOAT16", "INT8",  "UINT8",
        "INT16",   "UINT16",  "INT32", "UINT32",
    };
    printf("\n########################\n");
    printf("NetName: %s\n", m_netinfo->name);
    for (int s = 0; s < m_netinfo->stage_num; s++) {
      printf("---- stage %d ----\n", s);
      for (int i = 0; i < m_netinfo->input_num; i++) {
        auto shapeStr = shape_to_str(m_netinfo->stages[s].input_shapes[i]);
        printf("  Input %d) '%s' shape=%s dtype=%s scale=%g\n", i,
               m_netinfo->input_names[i], shapeStr.c_str(),
               dtypeMap[m_netinfo->input_dtypes[i]],
               m_netinfo->input_scales[i]);
      }
      for (int i = 0; i < m_netinfo->output_num; i++) {
        auto shapeStr = shape_to_str(m_netinfo->stages[s].output_shapes[i]);
        printf("  Output %d) '%s' shape=%s dtype=%s scale=%g\n", i,
               m_netinfo->output_names[i], shapeStr.c_str(),
               dtypeMap[m_netinfo->output_dtypes[i]],
               m_netinfo->output_scales[i]);
      }
    }
    printf("########################\n\n");
  }
};

class BMNNHandle : public ::sophon_stream::common::NoCopyable {
  bm_handle_t m_handle;
  int m_dev_id;

 public:
  BMNNHandle(int dev_id = 0) : m_dev_id(dev_id) {
    int ret = bm_dev_request(&m_handle, dev_id);
    assert(BM_SUCCESS == ret);
  }

  ~BMNNHandle() { bm_dev_free(m_handle); }

  bm_handle_t handle() { return m_handle; }

  int dev_id() { return m_dev_id; }
};

using BMNNHandlePtr = std::shared_ptr<BMNNHandle>;

class BMNNContext : public ::sophon_stream::common::NoCopyable {
  BMNNHandlePtr m_handlePtr;
  void* m_bmrt;
  std::vector<std::string> m_network_names;

 public:
  BMNNContext(BMNNHandlePtr handle, const char* bmodel_file)
      : m_handlePtr(handle) {
    bm_handle_t hdev = m_handlePtr->handle();
    m_bmrt = bmrt_create(hdev);
    if (nullptr == m_bmrt) {
      std::cout << "bmrt_create() failed!" << std::endl;
      exit(-1);
    }

    if (!bmrt_load_bmodel(m_bmrt, bmodel_file)) {
      std::cout << "load bmodel(" << bmodel_file << ") failed" << std::endl;
    }

    load_network_names();
  }

  ~BMNNContext() {
    if (m_bmrt != nullptr) {
      // bmrt_destroy(m_bmrt);
      m_bmrt = NULL;
    }
  }

  bm_handle_t handle() { return m_handlePtr->handle(); }

  void* bmrt() { return m_bmrt; }

  void load_network_names() {
    const char** names;
    int num;
    num = bmrt_get_network_number(m_bmrt);
    bmrt_get_network_names(m_bmrt, &names);
    for (int i = 0; i < num; ++i) {
      m_network_names.push_back(names[i]);
    }

    free(names);
  }

  std::string network_name(int index) {
    if (index >= (int)m_network_names.size()) {
      return "Invalid index";
    }

    return m_network_names[index];
  }

  std::shared_ptr<BMNNNetwork> network(const std::string& net_name) {
    return std::make_shared<BMNNNetwork>(m_bmrt, net_name);
  }

  std::shared_ptr<BMNNNetwork> network(int net_index) {
    assert(net_index < (int)m_network_names.size());
    return std::make_shared<BMNNNetwork>(m_bmrt, m_network_names[net_index]);
  }
};