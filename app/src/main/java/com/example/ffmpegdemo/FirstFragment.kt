package com.example.ffmpegdemo

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import androidx.navigation.fragment.findNavController
import com.example.ffmpegdemo.databinding.FragmentFirstBinding
import com.example.nativelib.NativeLib

/**
 * A simple [Fragment] subclass as the default destination in the navigation.
 */
class FirstFragment : Fragment() {
    companion object{
        const val REQUEST_CODE = 10001
    }

    private var _binding: FragmentFirstBinding? = null

    // This property is only valid between onCreateView and
    // onDestroyView.
    private val binding get() = _binding!!

    override fun onCreateView(
        inflater: LayoutInflater, container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        _binding = FragmentFirstBinding.inflate(inflater, container, false)
        return binding.root
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)

        binding.buttonFirst.setOnClickListener {
            findNavController().navigate(R.id.action_FirstFragment_to_SecondFragment)
        }
        binding.textviewFirst.text = NativeLib.ffmpegVersion()
        binding.btnDecodeVideoPlay.setOnClickListener{
            findNavController().navigate(R.id.action_FirstFragment_to_DecodeVideoPlayFragment)
        }
        binding.btnAudioPlay.setOnClickListener{
            findNavController().navigate(R.id.action_FirstFragment_to_DecodeAudioPlayFragment)
        }
    }

    override fun onDestroyView() {
        super.onDestroyView()
        _binding = null
    }

}